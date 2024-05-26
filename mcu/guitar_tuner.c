/*
 * Copyright (C) 2024 Petar Turukalo
 * SPDX-License-Identifier: GPL-2.0
 */
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/f4/nvic.h>
#include <libopencm3/cm3/cortex.h>
#include <stdio.h>
#include "adc.h"
#include "dsp.h"
#include "note.h"
#include "ssd1306.h"
#include "font.h"
#include "debug.h"

#define FRAME_LEN  FRAME_LEN_4096
/* The ADC regular data register data field is 16 bits wide, but the sample is 12 bits. */
#define ADC_DR_DATA_MASK 0x00000fff

/*
 * This is a circular buffer storing 2 frames worth of samples so that one frame can
 * be filled while the other full frame is being processed.
 */
static volatile float32_t samples[FRAME_LEN*2];
static volatile float32_t *volatile full_samples_frame = NULL;

/*
 * Store the converted sample in the next free slot in the samples circular buffer. 
 *
 * When a frame has been filled full_samples_frame is set to the first sample in the
 * filled frame, signalling that the frame is ready for processing (see processing_start()).
 */
void adc_isr(void) 
{
	static int i = 0;

	samples[i++] = convert_adc_u12_sample_to_s16(adc_read_regular(ADC1)&ADC_DR_DATA_MASK);

	/* If just finished filling a frame of samples. */
	if (i%FRAME_LEN == 0) {
		full_samples_frame = samples+(i-FRAME_LEN);
		if (i == FRAME_LEN*2)
			i = 0;
	}
}

/* Set timer 2 (TIM2) sampling rate to OVERSAMPLING_RATE. */
static void timer2_set_sampling_rate(void)
{
	/*
	 * The timer is running off APB1, which is 6 MHz, but because the APB1 prescaler is > 1,
	 * the timer clock frequencies are twice APB1, which is 12 MHz. This clock divider sets
	 * the counter clock frequency to twice the OVERSAMPLING_RATE (i.e. 12 MHz / clock_div = 2*OVERSAMPLING_RATE),
	 * because it takes a clock cycle to count from 0 to 1, and another clock cycle to overflow 
	 * from 1 back to 0: each update event takes 2 cycles, so with a clock rate of 2*OVERSAMPLING_RATE 
	 * there will be OVERSAMPLING_RATE update events. 
	 *
	 * Note also the counter clock is lowered rather than raising the timer period in order to save power.
	 */
	const int clock_div = 1500;
	timer_set_prescaler(TIM2, clock_div-1);
	timer_set_period(TIM2, 1);

	/* Trigger update event to load "preload" prescaler value set above into preload register proper. */
	timer_generate_event(TIM2, TIM_EGR_UG);
	timer_clear_flag(TIM2, TIM_EGR_UG);
}

/*
 * Configure timer 2 (TIM2) to send an update event as trigger output (TRGO) at a sampling rate of OVERSAMPLING_RATE.
 */
static void timer_init(void)
{
	rcc_periph_clock_enable(RCC_TIM2);

	/* Below 2 lines put the timer in upcounting mode. */
	timer_set_alignment(TIM2, TIM_CR1_CMS_EDGE);
	timer_direction_up(TIM2);
	/* Generate an update event when timer upcounts from 0 to the period set with timer_set_period(). */
	timer_update_on_overflow(TIM2);
	timer_enable_update_event(TIM2);
	/* Send update event as trigger output (TRGO).  */
	timer_set_master_mode(TIM2, TIM_CR2_MMS_UPDATE);
	timer2_set_sampling_rate();
}

/*
 * Initialise the ADC to do a single conversion on regular channel ADC1 (pin PA1) when externally
 * triggered by the timer 2 (TIM2) trigger output (TRGO). End of conversion interrupt is enabled
 * and handled by adc_isr(). 
 */
static void adc_init(void)
{
	const uint8_t adc_channel = 1;

	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_ADC1);

	gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO1);

	adc_set_clk_prescale(ADC_CCR_ADCPRE_BY8);  /* Use slowest clock to save power. */
	adc_set_resolution(ADC1, ADC_CR1_RES_12BIT);
	adc_set_single_conversion_mode(ADC1);
	adc_set_regular_sequence(ADC1, 1, (uint8_t *)&adc_channel);
	adc_enable_eoc_interrupt(ADC1);
	nvic_enable_irq(NVIC_ADC_IRQ);
	adc_enable_external_trigger_regular(ADC1, ADC_CR2_EXTSEL_TIM2_TRGO, ADC_CR2_EXTEN_RISING_EDGE);

	adc_power_on(ADC1);
}

/*
 * Initialise the sampler to sample at an OVERSAMPLING_RATE sampling rate.
 * Call sampler_start() to start sampling.
 *
 * Hardware timing is used to achieve the sampling rate, with a timer driving 
 * the ADC rather than setting up the ADC in continuous mode because its choice of
 * sampling rate is more flexible. The very precise timing provided by hardware timing 
 * is needed, e.g. over manually starting a conversion and then sleeping in a loop, 
 * otherwise the signal may not be restored accurately. 
 */
static void sampler_init(void)
{
	timer_init();
	adc_init();
}

/* 
 * Start the sampler initialised in sampler_init(), which will start the triggering
 * of ADC interrupts and servicing of them with adc_isr().
 */
static void sampler_start(void)
{
	timer_enable_counter(TIM2);
}

/*
 * Display the closest note (or a question mark for no such closest note) and a slider depicting 
 * how close the detected frequency is to the closest note. On the slider are two tics, one smaller 
 * in the centre to mark the point on the slider that is exactly in tune with the closest note, 
 * acting as a reference for the other bigger tic which marks the detected frequency and how far 
 * away it is from the closest note in cents. One pixel in the slider is one cent. 
 *
 * For example, the display could look like the following (not to scale).
 *
 *              A#2
 *           |  
 *        ---|---|-------
 *           |
 *	         ^ smaller tic
 *	     ^ larger tic
 *
 * Note the larger tic can overlap the smaller tic.
 */
static void display_note_and_slider(float32_t frequency)
{
	const int centre_pixel = GDDRAM_PIXEL_WIDTH/2;
	int text_centre_align_col;
	int slider_centre_align_col;
	const int slider_row = 47;
	/* Pixels above and below the slider line, on each side. */
	const int centre_tic_half_height = 4;  
	const int detect_tic_half_height = 8;  
	int detect_tic_col;
	struct note_freq *nf = nearest_note(frequency);

	if (!nf) 
		nf = &null_nf;
	printf("note name = %s, note freq = %.3f, detect freq = %.3f\n", nf->note_name, nf->frequency, frequency);
	gddram_mcu_buf_zero();

	/* Draw note name text. */
	text_centre_align_col = centre_pixel - ((FONT_PIXEL_WIDTH*strlen(nf->note_name))/2);
	gddram_mcu_buf_write_text(nf->note_name, (struct write_coord){0,text_centre_align_col});

	/* Draw slider. */
	slider_centre_align_col = centre_pixel - CENTS_IN_HALF_SEMITONE;
	gddram_mcu_buf_write_horizontal_line((struct write_coord){slider_row,slider_centre_align_col}, 
					     /* Up to 50 cents either side of the centre. */
					     CENTS_IN_HALF_SEMITONE+1+CENTS_IN_HALF_SEMITONE);
	/* Draw slider centre tic. */
	gddram_mcu_buf_write_vertical_line((struct write_coord){slider_row-centre_tic_half_height,centre_pixel}, 
					   2*centre_tic_half_height + 1);
	/* Draw slider detected frequency tic. */
	if (nf != &null_nf) {
		detect_tic_col = centre_pixel + cents_difference(frequency, nf);
		gddram_mcu_buf_write_vertical_line((struct write_coord){slider_row-detect_tic_half_height,detect_tic_col}, 
						   2*detect_tic_half_height + 1);
	}
	ssd1306_fill_gddram();
}

static void display_question_mark(void)
{
	display_note_and_slider(0);
}

static void processing_init(void)
{
	counter_init();
	samples_to_freq_bin_magnitudes_init(FRAME_LEN);
	ssd1306_init_i2c(SSD1306_I2C_SLAVE_ADDR_LOW);
	ssd1306_init();
	/* Show a question mark while the very first frame of samples is being collected. */
	display_question_mark();
}

/*
 * Continuously wait for a frame of samples to be filled, then processing the full frame for
 * a detected closest note and showing it on the display. Because the sampling rate (OVERSAMPLING_RATE)
 * is 4000 and the frame length (FRAME_LEN) is 4096, it will take 4096/4000 = 1.024 seconds to
 * fill a frame. The processing of a frame from testing then takes around 0.092 seconds.
 */
static void processing_start(void)
{
	float32_t *freq_bin_magnitudes;
	int max_bin_ind;
	float32_t frequency; 
	// TODO rm?
	uint32_t prev_proc_start, proc_start, proc_end; 
	
	proc_start = counter_count();
	for (;;) {
		/* Wait for sampler to fill frame. See adc_isr(). */
		do {
			__asm__("wfi");
		} while (!full_samples_frame);

		prev_proc_start = proc_start;
		proc_start = counter_count();

		/* DSP. */
		freq_bin_magnitudes = samples_to_freq_bin_magnitudes((const float32_t *)full_samples_frame, FRAME_LEN);
		harmonic_product_spectrum(freq_bin_magnitudes, FRAME_LEN);
		max_bin_ind = max_bin_index(freq_bin_magnitudes, FRAME_LEN);
		frequency = bin_index_to_freq(max_bin_ind, bin_width(FRAME_LEN));

		/*
		 * Only display a note if the reading is strong enough, in order to filter out readings where there is
		 * no actual note being played. From testing, the resting max magnitude when there is no sound being
		 * made is e+17 (because the ADC is quite noisy), and when you play a note it will start at around 
		 * e+22 to e+25 and then fade out / decline back to e+17. Be wary that this e+17 "noise floor" is 
		 * when powering the MCU off a battery via the MCU's 5V pin: a dirtier power source, such as the
		 * ST-Link, will have a higher noise floor, e.g. e+20, and so this threshold won't work and also
		 * a note when played won't "hold" (display on screen) as long.
		 */
		if (freq_bin_magnitudes[max_bin_ind] >= 1.3e+18) 
			display_note_and_slider(frequency);
		else
			display_question_mark();

		proc_end = counter_count();
		printf("max mag %e, ", freq_bin_magnitudes[max_bin_ind]);
		printf("frame fill time = %d, proc time = %d\n", proc_start-prev_proc_start, proc_end-proc_start);

		full_samples_frame = NULL;
	}
}

int main(void)
{
	/*
	 * Sysclk runs off HSI at reset. Switch sysclk to run off the 25 MHz HSE on my board
	 * boosted to 96 MHz via a PLL, and turn off HSI. HSE because it's more stable/accurate 
	 * than HSI, preferred for the ADC, and boosted by a PLL for a higher frequency needed 
	 * for processing samples.
	 */
	struct rcc_clock_scale rcc_hse_25mhz_3v3_copy = rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_96MHZ];

	/* Use slowest APB clocks to save power. */
	rcc_hse_25mhz_3v3_copy.ppre1 = RCC_CFGR_PPRE_DIV_16;
	rcc_hse_25mhz_3v3_copy.ppre2 = RCC_CFGR_PPRE_DIV_16;
	rcc_hse_25mhz_3v3_copy.apb1_frequency = 6000000;
	rcc_hse_25mhz_3v3_copy.apb2_frequency = 6000000;

	rcc_clock_setup_pll(&rcc_hse_25mhz_3v3_copy);
	/* Peripheral clocks must be enabled after this point. */

	/*
	 * Disable display peripheral clocks during sleep mode because they're
	 * only needed when processing.
	 */
	RCC_AHB1LPENR &= ~RCC_AHB1LPENR_GPIOBLPEN;
	RCC_APB1LPENR &= ~RCC_APB1LPENR_I2C1LPEN;

	uart_init();
	cm_enable_interrupts();
	sampler_init();
	processing_init();
	sampler_start();
	processing_start();
}

