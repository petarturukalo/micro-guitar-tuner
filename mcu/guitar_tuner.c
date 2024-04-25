#include <stdio.h>
#include <string.h>
#include <pico/stdio_usb.h>
#include <pico/time.h>/* TODO rm? */
#include <hardware/adc.h>
#include <hardware/i2c.h>
#include <pico/multicore.h>
#include "adc.h"
#include "dsp.h"
#include "note.h"
#include "ssd1306.h"
#include "font.h"

#define FRAME_LEN FRAME_LEN_4096

/* Pin number of GPIO which ADC0 uses. */
#define ADC0_GPIO_PIN 26
#define ADC0_CHANNEL 0
/* Hz rate of the clock driving the ADC. */
#define ADC_CLK_RATE_HZ 48000000

static void adc_set_sampling_rate(int sampling_rate)
{
	/*
	 * The ADC has a max sampling rate of 500 ksps, which is only 
	 * the case because it is driven by a clock of rate ADC_CLK_RATE_HZ
	 * and capturing a sample (the sample period) takes 96 clock cycles: 
	 * ADC_CLK_RATE_HZ / 96 = 500 ksps.
	 *
	 * Setting a clock divider (clkdiv) with adc_set_clkdiv() changes the sample 
	 * period from 96 clock cycles to clkdiv + 1 clock cycles. Thus in general the
	 * sampling rate is calculated as ADC_CLK_RATE_HZ/(clkdiv+1) = sampling_rate.
	 * This equation is rearranged to calculate clkdiv from sampling_rate.
	 */
	float clkdiv = ADC_CLK_RATE_HZ/sampling_rate - 1;
	adc_set_clkdiv(clkdiv);
}

/*
 * TODO explain why this stores 2 frames. so processing core can process while other
 * frame gets filled
 */
/* TODO need volatile for shared? volatile before or after static? */
static volatile float32_t samples[FRAME_LEN*2];

/* TODO ISR coding style? what's needed for robust ISR? volatile (or that's just for mmio)? certainly not printing */
/* TODO worry about timing here and making sure it's stored before the next interrupt.
 * RTOS? and for locking variables shared between cores?*/
/* TODO bother explaining? */
static void adc_isr(void) 
{
	static int i = 0;

	/* Call to adc_fifo_get() will drain the FIFO and clear the interrupt. */
	samples[i++] = convert_adc_u12_sample_to_s16(adc_fifo_get()&ADC_FIFO_VAL_BITS);

	/* 
	 * If just finished filling a frame of samples.
	 *
	 * Note variable i will never be 0 because of the i++ so 0%FRAME_LEN == 0 (true)
	 * will never accidentally enter this block.
	 */
	if (i%FRAME_LEN == 0) {
		/* TODO if use FreeRTOS SMP can't use these multicore fifo fns directly? */
		/* Send start index of frame to processing core. */
		/* TODO safe to use in IRQ? */
		multicore_fifo_push_blocking(i-FRAME_LEN);
		if (i == FRAME_LEN*2)
			i = 0;
	}
}

/*
 * Initialise the sampler to read samples from pin ADC0 at the
 * given sampling rate. Call sampler_start() to start sampling.
 * 
 * Samples are captured in interrupt driven free-running sampling mode,
 * which utilises hardware timing to achieve the sampling rate. 
 * This very precise timing is needed otherwise the signal may not be
 * restored accurately.
 *
 * WARNING do not use sampling rates less than ~750 because from testing
 * they do not work as intended and give a spurious sampling rate.
 */
static void sampler_init(int sampling_rate)
{
	adc_init();
	/* Configure ADC to read from the ADC0 pin. */
	adc_gpio_init(ADC0_GPIO_PIN);
	adc_select_input(ADC0_CHANNEL);

	adc_set_sampling_rate(sampling_rate);
	/* TODO could wake up after 4 samples (FIFO is full) to not have to wake up as often? as long
	 * as sampling rate is divisible by 4. probably better to do to sleep longer and save power */
	/* TODO rp2040 datasheet 4.9.2.4. says FIFO is 8 entries? */
	adc_fifo_setup(true, 
		       false,	/* Only for DMA. */
		       1,	/* Have IRQ trigger for each sample. */
		       false,	/* TODO set to true and check ERR in ISR? */
		       false);	/* Only for DMA. */

	irq_set_exclusive_handler(ADC_IRQ_FIFO, adc_isr);
	/* Enable the ADC0 interrupt in the NVIC interrupt controller. */
	irq_set_enabled(ADC_IRQ_FIFO, true);

	adc_irq_set_enabled(true);
}

/*
 * Spin waiting to service ADC interrupts with adc_isr(). See also sampler_init().
 */
static void sampler_start(void)
{
	adc_run(true);

	for (;;)
		__asm__("wfi");
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
	/* TODO rm */
	printf("note = %s, ref = %.3f, actual = %.3f\n", nf->note_name, nf->frequency, frequency);
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

/* TODO make sure this core doesn't receive ADC interrupts */
/* TODO explain all of what this does when finished and displaying to screen */
/* TODO timing. need to finish processing before sampling core catches back up */
static void processing_core(void)
{
	int frame_start_index;
	float32_t *framed_samples, *freq_bin_magnitudes;
	int max_bin_ind;
	float32_t frequency; 

	samples_to_freq_bin_magnitudes_init(FRAME_LEN);
	ssd1306_init_i2c(i2c0, 12, 13, SSD1306_I2C_SLAVE_ADDR_LOW);
	ssd1306_init();
	/* Show a question mark. */
	display_note_and_slider(0);

	/* TODO make sure cores are synchronised and other core waits for this to get here. */
	for (;;) {
		/* Wait for sampling core to finish filling a frame. */
		frame_start_index = multicore_fifo_pop_blocking();
		framed_samples = samples + frame_start_index;

		absolute_time_t start = get_absolute_time();//TODO rm

		/* DSP. */
		freq_bin_magnitudes = samples_to_freq_bin_magnitudes_f32(framed_samples, FRAME_LEN);
		harmonic_product_spectrum(freq_bin_magnitudes, FRAME_LEN);
		max_bin_ind = max_bin_index(freq_bin_magnitudes, FRAME_LEN);
		frequency = bin_index_to_freq(max_bin_ind, bin_width(FRAME_LEN));

		printf("max mag %e, ", freq_bin_magnitudes[max_bin_ind]);
		display_note_and_slider(frequency);
		printf("time %llu\n", absolute_time_diff_us(start, get_absolute_time()));//TODO rm
		/* 
		 * TODO don't report note if it's too quiet? so don't get spurious results 
		 * when nothing playing. try out by printing max value of mag
		 */
	}
}

/* TODO explain high-level interaction between the cores here or somewhere? */
int main(void)
{
	stdio_usb_init();/*TODO for one core? both? none?*/

	multicore_launch_core1(processing_core);

	/* Core 0 (this core) is the sampling core. */

	sampler_init(OVERSAMPLING_RATE);
	sampler_start();
}

