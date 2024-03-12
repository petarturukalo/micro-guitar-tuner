#include <stdio.h>
#include <pico/stdio_usb.h>
#include <pico/time.h>/* TODO rm? */
#include <hardware/adc.h>
#include "adc.h"

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

/* TODO rm */
int16_t samples[4096];
unsigned int i = 0;

/* TODO ISR coding style? what's needed for robust ISR? certainly not printing */
/* TODO worry about timing here and making sure it's stored before the next interrupt.
 * RTOS? and for locking variables shared between cores?*/
/* TODO bother explaining? */
static void adc_isr(void) 
{
	/* TODO explain also drains FIFO and clears interrupt. */
	samples[i++] = adc_fifo_get()&ADC_FIFO_VAL_BITS;
}

/*
 * Initialise the sampler to read samples from pin ADC0 at the
 * given sampling rate. Call sampler_start() to start sampling.
 * 
 * Samples are captured in interrupt driven free-running sampling mode,
 * which utilises hardware timing to achieve the sampling rate. 
 * This very precise timing is needed otherwise the restored signal 
 * may have spurious results.
 *
 * WARNING do not use sampling rates less than ~750 because from testing
 * they do not work as intended and give a random sampling rate.
 */
static void sampler_init(int sampling_rate)
{
	adc_init();
	/* Configure ADC to read from the ADC0 pin. */
	adc_gpio_init(ADC0_GPIO_PIN);
	adc_select_input(ADC0_CHANNEL);

	adc_set_sampling_rate(sampling_rate);
	/* TODO could wake up after 4 samples (FIFO is full) to not have to wake up as often? as long
	 * as sampling rate is divisible by 4 */
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
	/* TODO rm */
	for (int countdown = 17; countdown > 0; --countdown) {
		printf("countdown %d\n", countdown);
		sleep_ms(1000);
	}

	absolute_time_t tm = make_timeout_time_ms(1000);
	adc_run(true);
	while (get_absolute_time() < tm)
		;
	adc_run(false);

	printf("i=%d\n", i);
	for (int i = 0; i < 4096; ++i) {
		printf("%.9f, ", convert_adc_u12_sample_to_s16(samples[i]));
		if (i%10 == 0)
			printf("\n");
	}
	for (;;)
		;
	/*for (;;)*/
		/*__asm__("wfi");*/
}

int main(void)
{
	stdio_usb_init();/*TODO for one core? both? none?*/
	/* TODO make sure second core doesn't receive ADC interrupts */
	/* TODO explain high-level interaction between the cores here? */

	/* TODO this is sampling core. start other processing core and sleep in that
	 * until woken up by sampling core*/
	sampler_init(4096);  /*TODO define */
	sampler_start();
}
