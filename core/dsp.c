#include <dsp/filtering_functions.h>
#include <dsp/transform_functions.h>
#include <dsp/complex_math_functions.h>
#include "dsp.h"
#include "filter_coeffs.c"

/* 
 * TODO change float32_t to q notation later if find can't use it on cortex-m0+ 
 * (most likely will need to use q notation) 
 */

#define MAX_NR_BINS (MAX_FRAME_LEN/2)

static void s16_array_to_f32(const int16_t *src, float32_t *dest, int len)
{
	for (int i = 0; i < len; ++i) 
		/* TODO need to scale range of short up to range of float? */
		dest[i] = (float32_t)src[i];
}

float32_t *guitar_tuner_dsp(const int16_t *samples, enum frame_length frame_len)
{
	static float32_t float_samples[MAX_FRAME_LEN];
	static float32_t fir_state[NR_TAPS+MAX_FRAME_LEN-1];
	arm_fir_instance_f32 fir_instance;
	static float32_t filtered_samples[MAX_FRAME_LEN]; 
	arm_rfft_fast_instance_f32 fft_instance;
	static float32_t fft_complex_nrs[MAX_FRAME_LEN];
	static float32_t freq_bin_magnitudes[MAX_NR_BINS];

	s16_array_to_f32(samples, float_samples, frame_len);
	/* 
	 * TODO how to test this? 
	 * with note recording higher than the highest supported note / cutoff freq? 
	 */
	/* Apply band-pass filter. */
	arm_fir_init_f32(&fir_instance, NR_TAPS, filter_coefficients, fir_state, frame_len);
	arm_fir_f32(&fir_instance, float_samples, filtered_samples, frame_len);
	/* 
	 * TODO don't do this every time? only do it on the first time a size is used? 
	 * look into source for whether need to
	 */
	/* Convert from time domain to frequency domain. */
	arm_rfft_fast_init_f32(&fft_instance, frame_len);
	arm_rfft_fast_f32(&fft_instance, filtered_samples, fft_complex_nrs, 0);
	/* TODO worry about different format / caveats when swap to q notation? */
	/* TODO zero first mag because it's mag of 2 reals DC and nyquist? */
	/* TODO can the "squared" part of this push the numbers out of range and overflow? */
	/* Get the energy of the spectra. */
	arm_cmplx_mag_squared_f32(fft_complex_nrs, freq_bin_magnitudes, MAX_NR_BINS);
	return freq_bin_magnitudes;
}

int nr_bins(enum frame_length frame_len)
{
	return frame_len/2;
}

int bandwidth(void)
{
	return OVERSAMPLING_RATE/2;
}

int bin_width(enum frame_length frame_len)
{
	/*
	 * Note the return is purposefully int here because the current combinations 
	 * of OVERSAMPLING_RATE and frame lengths all have an integer bin width. 
	 * If this changes in the future (e.g. because OVERSAMPLING_RATE was changed) 
	 * then the types here may need to change to floating point.
	 */
	return bandwidth()/nr_bins(frame_len);
}
