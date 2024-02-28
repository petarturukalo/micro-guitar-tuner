#include <dsp/filtering_functions.h>
#include <dsp/transform_functions.h>
#include <dsp/complex_math_functions.h>
#include "dsp.h"
#include "filter_coeffs.c"
#include <stdbool.h>

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

float32_t *samples_to_freq_bin_magnitudes(const int16_t *samples, enum frame_length frame_len)
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
	/* 
	 * Get the energy of the spectra. Use regular mag over mag squared because the numbers mag
	 * squared ouput are too big and cause the result of HPS to overflow and give wrong results. 
	 */
	arm_cmplx_mag_f32(fft_complex_nrs, freq_bin_magnitudes, MAX_NR_BINS);
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

/* Get the index of the bin which the frequency falls into. */
/* TODO make take frame_len instead of binwidth? */
/* TODO make non-static and run unit tests? and for other fns here? */
static int freq_bin_index(float32_t frequency, int binwidth)
{
	return (frequency/binwidth)+1;
}

/*
 * Get the maximum float value in a subarray of floats. The subarray starts
 * at index start_index (inclusive) and ends at index stop_index (inclusive).
 */
static float32_t max_subarray_f32(float32_t *floats, int start_index, int stop_index)
{
	float32_t max = floats[start_index];

	for (int i = start_index+1; i <= stop_index; ++i) {
		if (floats[i] > max)	
			max = floats[i];
	}
	return max;
}

/*
 * This implements A. Michael Noll's Harmonic Product Spectrum formula
 *
 *	HPS(f) = Π_{k=1}^K |F(k*f)|
 *	
 * which takes as input a frequency f, and outputs the product of the first K harmonic
 * magnitudes. Note the input frequency is the very first (k=1) harmonic.
 *
 * TODO add a "see paper referenced in resources" after resources added to main readme
 * TODO how to test the parts of this function separately to a note file source?
 */
void harmonic_product_spectrum(float32_t *freq_bin_magnitudes, enum frame_length frame_len)
{
	/* Number of harmonics can't be too high otherwise the new product value will overflow. */
	const int nharmonics = 4;  
	const int nbins = nr_bins(frame_len);
	int i;  /* Bin index. */

	/*
	 * Skip the frequencies below the lowest note.
	 * TODO replace 16 with define later on and then reword this comment
	 */
	i = freq_bin_index(16, bin_width(frame_len));
	for (bool finished = false; !finished; ++i) {
		/* 
		 * Don't bother if the frequency ranges of harmonics overlap (see below harmonic inner loop 
		 * comment for more info). But this shouldn't happen anyway because we're skipping the lower
		 * frequencies. TODO unless use a small enough frame len which bumps up the bin width?
		 * TODO confirm
		 */
		if (i < nharmonics)
			continue;
		/* Start at 2 because the current bin is the 1st harmonic. */
		for (int harmonic = 2; harmonic <= nharmonics; ++harmonic) {
			/* 
			 * We can't know the exact frequency of the peak, only that its frequency
			 * is within the frequency range of the current bin. Let the frequency range
			 * of the current bin start at start_freq and end at end_freq, which has a bandwidth
			 * of bin_width() Hz. If the peak is at start_freq then the peak of the 2nd harmonic
			 * will be at 2*start_freq, and if the peak is at end_freq then the peak of the 2nd
			 * harmonic will be at 2*end_freq, but because end_freq can be written in terms of 
			 * start_freq as end_freq = start_freq + bin_width(), the peak of the 2nd harmonic 
			 * becomes 2*end_freq = 2*(start_freq + bin_width()) = 2*start_freq + 2*bin_width(). 
			 * The bandwidth of the 2nd harmonic frequency range is then its end minus its start:
			 * (2*start_freq + 2*bin_width()) - 2*start_freq = 2*bin_width.
			 *
			 * In general, the frequency range that the peak of the kth harmonic is in has a bandwidth 
			 * of k*bin_width(). This mean the peak could be in one of many bins. To find this one bin, 
			 * the below picks the bin in this bandwidth which has the max (peak) magnitude as the bin
			 * of the kth harmonic peak.
			 */
			/* TODO confirm */
			int harmonic_highest_bin_index = harmonic*i;
			/* TODO still try to find max if lower is in bound? doesn't matter anyway because of lowpass filter? */
			if (harmonic_highest_bin_index >= nbins) {
				/* 
				 * The frequency of the harmonic is higher than the total bandwidth and
				 * not in the array, so can't use it in product. If it's the 2nd harmonic,
				 * the even higher (3rd, 4th, etc.) harmonics won't be in the bandwidth either, 
				 * and because the bins that follow span higher frequencies than the current bin, 
				 * their 2nd harmonics also won't be in bounds, so terminate.
				 */
				if (harmonic == 2)
					finished = true;
				break;
			}
			/* The subarray spans k (harmonic) bins (remembering the k*bin_width() bandwidth from above) */
			freq_bin_magnitudes[i] *= max_subarray_f32(freq_bin_magnitudes, harmonic_highest_bin_index - (harmonic-1),
											harmonic_highest_bin_index);
		}
	}
}

