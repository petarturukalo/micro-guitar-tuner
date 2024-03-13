#include <dsp/filtering_functions.h>
#include <dsp/transform_functions.h>
#include <dsp/complex_math_functions.h>
#include <dsp/statistics_functions.h>
#include "dsp.h"
#include "note.h"
#include "filter_coeffs.c"
#include <stdbool.h>

/* 
 * TODO change float32_t to q notation later if find can't use it on cortex-m0+ 
 * (most likely will need to use q notation) 
 * TODO pico might actually have single precision float support available through
 * its SDK runtime libraries, it's just that pure Cortex-M0+ doesn't
 */

float32_t *samples_to_freq_bin_magnitudes_f32(const float32_t *samples, enum frame_length frame_len)
{
	static float32_t fir_state[NR_TAPS+MAX_FRAME_LEN-1];
	arm_fir_instance_f32 fir_instance;
	static float32_t filtered_samples[MAX_FRAME_LEN]; 
	arm_rfft_fast_instance_f32 fft_instance;
	static float32_t fft_complex_nrs[MAX_FRAME_LEN];
	static float32_t freq_bin_magnitudes[MAX_NR_BINS];

	/* 
	 * TODO how to test this? 
	 * with sine waves below and above the cutoff
	 */
	/* Apply band-pass filter. */
	/* TODO only init once? */
	arm_fir_init_f32(&fir_instance, NR_TAPS, filter_coefficients, fir_state, frame_len);
	arm_fir_f32(&fir_instance, samples, filtered_samples, frame_len);
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

/* TODO is there a CMSIS-DSP fn for this? */
static void s16_array_to_f32(const int16_t *src, float32_t *dest, int len)
{
	for (int i = 0; i < len; ++i) 
		dest[i] = (float32_t)src[i];
}

float32_t *samples_to_freq_bin_magnitudes_s16(const int16_t *samples, enum frame_length frame_len)
{
	static float32_t float_samples[MAX_FRAME_LEN];
	s16_array_to_f32(samples, float_samples, frame_len);
	return samples_to_freq_bin_magnitudes_f32(float_samples, frame_len);
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

enum frame_length frame_length_from_bin_width(int binwidth)
{
	enum frame_length frame_len = OVERSAMPLING_RATE/binwidth;

	/* TODO support others? unless they're removed later anyway */
	if (frame_len == FRAME_LEN_2048 || frame_len == FRAME_LEN_4096)
		return frame_len;
	return 0;
}

/* 
 * Round a floating point number to the nearest integer. 
 * If half way between then rounds down.
 */
static int Round(float32_t n)
{
	int floored = n/1;
	float32_t fractional_part = n - floored;

	if (fractional_part == 0.5) 
		return floored;
	return fractional_part > 0.5 ? floored+1 : floored;
}

int freq_to_bin_index(float32_t frequency, int binwidth)
{
	/*
	 * The calculation here to round to the nearest bin index comes from testing
	 * and seeing which bin sine waves at particular frequencies fall into. 
	 * See assert_sine_wave_freq_to_bin_index() for these tests (see also the sine wave 
	 * data that it tests on).
	 */
	return Round(frequency/binwidth);
}

float32_t bin_index_to_freq(int bin_index, int binwidth)
{
	return bin_index*binwidth;
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
 */
void harmonic_product_spectrum(float32_t *freq_bin_magnitudes, enum frame_length frame_len)
{
	const int nbins = nr_bins(frame_len);
	int i;  /* Bin index. */

	/* Skip the frequencies below the lowest note. */
	i = freq_to_bin_index((int)lowest_note_frequency(), bin_width(frame_len));
	for (bool finished = false; !finished; ++i) {
		/* Start at 2 because the current bin is the 1st harmonic. */
		for (int harmonic = 2; harmonic <= NHARMONICS; ++harmonic) {
			/* 
			 * The frequency of current bin, bin at index i, is i*bin_width(). The frequency of
			 * the kth harmonic is k times the frequency of the current bin, k*(i*bin_width()).
			 * But this is also the same as (k*i)*bin_width(), the frequency of bin at index k*i,
			 * so the bin index of the harmonic is k*i (or harmonic*i).
			 *
			 * Note this is equivalent to downsampling/compression at a factor k for each harmonic
			 * and then taking the product at index i because the compression is shifting the bin
			 * index of the harmonic from index k*i down to index i.
			 */
			int harmonic_bin_index = harmonic*i;
			if (harmonic_bin_index >= nbins) {
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
			freq_bin_magnitudes[i] *= freq_bin_magnitudes[harmonic_bin_index];
		}
	}
}

/* 
 * Wrapper for arm_max_f32() so 1. user of this lib doesn't have to write their own max 
 * function, and 2. to reuse arm_max_f32() internally and not prompt user to call arm_max_f32() 
 * directly when looking to not write their own max function because the CMSIS DSP object files 
 * are internal to this lib.
 */
int max_bin_index(float32_t *freq_bin_magnitudes, enum frame_length frame_len)
{
	float32_t max;
	uint32_t max_index;

	arm_max_f32(freq_bin_magnitudes, nr_bins(frame_len), &max, &max_index);
	return max_index;
}
