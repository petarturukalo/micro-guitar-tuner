/*
 * Copyright (C) 2024 Petar Turukalo
 * SPDX-License-Identifier: GPL-2.0
 */
#include <dsp/filtering_functions.h>
#include <dsp/transform_functions.h>
#include <dsp/complex_math_functions.h>
#include <dsp/statistics_functions.h>
#include "dsp.h"
#include "note.h"
#include <stdbool.h>

extern const float32_t filter_coefficients[NR_TAPS];

static arm_fir_decimate_instance_f32 fir_decimate_instance;
static arm_rfft_fast_instance_f32 fft_instance;

void samples_to_freq_bin_magnitudes_init(enum frame_length frame_len)
{
	static float32_t fir_state[NR_TAPS+(OVERSAMPLING_FACTOR*MAX_FRAME_LEN)-1];
	arm_fir_decimate_init_f32(&fir_decimate_instance, NR_TAPS, OVERSAMPLING_FACTOR, filter_coefficients, 
				  fir_state, OVERSAMPLING_FACTOR*frame_len);
	arm_rfft_fast_init_f32(&fft_instance, frame_len);
}

float32_t *samples_to_freq_bin_magnitudes(float32_t *samples, enum frame_length frame_len)
{
	/*
	 * Each processing step below interleaves between using `buf` and `samples` as input/output
	 * buffers instead of allocating memory for each, in order to save MCU RAM space.
	 */
	static float32_t buf[MAX_FRAME_LEN]; 
	float32_t *filtered_samples = buf;
	float32_t *fft_complex_nrs, *freq_bin_magnitudes;

	/* Apply band-pass filter and decimate down from the OVERSAMPLING_RATE to SAMPLING_RATE. */
	arm_fir_decimate_f32(&fir_decimate_instance, samples, filtered_samples, OVERSAMPLING_FACTOR*frame_len);
	/* Convert from time domain to frequency domain. */
	fft_complex_nrs = samples;
	arm_rfft_fast_f32(&fft_instance, filtered_samples, fft_complex_nrs, 0);
	/* 
	 * Zero the first complex number because it's the DC offset and value at the Nyquist frequency 
	 * masquerading as a complex number.
	 */
	fft_complex_nrs[0] = fft_complex_nrs[1] = 0;
	/* 
	 * Get the energy of the spectra. Use regular mag over mag squared because the numbers mag
	 * squared ouput are too big and cause the result of HPS to overflow and give wrong results. 
	 */
	freq_bin_magnitudes = buf;
	arm_cmplx_mag_f32(fft_complex_nrs, freq_bin_magnitudes, MAX_NR_BINS);
	return freq_bin_magnitudes;
}

int nr_bins(enum frame_length frame_len)
{
	return frame_len/2;
}

int bandwidth(int sampling_rate)
{
	return sampling_rate/2;
}

float32_t bin_width(enum frame_length frame_len, int sampling_rate)
{
	return bandwidth(sampling_rate)/(float32_t)nr_bins(frame_len);
}

int freq_to_bin_index(float32_t frequency, float32_t binwidth)
{
	/*
	 * The calculation here to round to the nearest bin index comes from testing
	 * and seeing which bin sine waves at particular frequencies fall into. 
	 * See assert_sine_wave_freq_to_bin_index() for these tests (see also the sine wave 
	 * data that it tests on).
	 */
	return round(frequency/binwidth);
}

float32_t bin_index_to_freq(int bin_index, float32_t binwidth)
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
 * See also the Harmonic Product Spectrum paper by A. Michael Noll, linked to in the 
 * "Resources" section of the top-level README.
 */
void harmonic_product_spectrum(float32_t *freq_bin_magnitudes, enum frame_length frame_len, 
			       int sampling_rate)
{
	const int nbins = nr_bins(frame_len);
	int i;  /* Bin index. */

	/* Skip the frequencies below the lowest note. */
	i = freq_to_bin_index((int)lowest_note_frequency(), bin_width(frame_len, sampling_rate));
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

int max_bin_index(float32_t *freq_bin_magnitudes, enum frame_length frame_len)
{
	float32_t max;
	uint32_t max_index;

	arm_max_f32(freq_bin_magnitudes, nr_bins(frame_len), &max, &max_index);
	return max_index;
}
