#include <stdio.h>  /* For printf(). */
#include <pico/stdio_usb.h>
#include <pico/time.h>/* TODO rm? */
#include <hardware/adc.h>
#include "adc.h"

int main(void)
{
	stdio_usb_init();

	/* TODO defines, etc.? */
	adc_init();
	adc_gpio_init(26);
	adc_select_input(0);

	uint16_t samples[4096];

	for (int i = 15; i > 0; i--) {
		printf("Countdown %d...\n", i);
		sleep_ms(1000);
	}
	/* collect samples at sample rate */
	for (int i = 0; i < 4096; ++i){
		/* TODO the sampling rate is very important and must be done at 100% accuracy, with no printfs in each
		 * iter otherwise the audio will be warped and the frequency will be wrong. */
		samples[i] = adc_read();
		/* Sleep for 4096 sample rate 
		 * TODO in actual app need to account for time spent doing above computation*/
		sleep_us(244);
	}
	/* print vals */
	for (int i = 0; i < 4096; ++i) {
		float val = convert_adc_u12_sample_to_s16(samples[i]);
		printf("%.9g, ", val);
		if (i%10 == 0 && i > 0) 
			printf("\n");
	}
	printf("\n");
	while (1)
		;
	/* TODO return code doesn't matter? */
	return 0;
}
