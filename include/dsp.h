/*
 * Copyright (C) 2024 Petar Turukalo
 * SPDX-License-Identifier: GPL-2.0
 */
#ifndef DSP_H
#define DSP_H

#include <stdint.h>
#include <arm_math_types.h>

/*
 * SAMPLING_RATE is the sampling rate after decimation down from the OVERSAMPLING_RATE. 
 * Oversampling is done to prevent some aliasing as it allows capture of an OVERSAMPLING_FACTOR times 
 * larger frequency bandwidth than that of the SAMPLING_RATE. These extra frequencies above 
 * the Nyquist frequency of the SAMPLING_RATE, which would have caused aliasing when sampling
 * directly at the SAMPLING_RATE, can be low-pass filtered out before decimation.
 *
 * Although it's the decimation that introduces the aliasing of the higher captured frequencies
 * in the first place, it is done in order to get the sampling rate closer to the max frame 
 * length to meet a reasonable frequency/time resolution tradeoff (see explanation at bin_width()).
 */
#define SAMPLING_RATE  SAMPLING_RATE_FROM_MAKEFILE
#define OVERSAMPLING_FACTOR  OVERSAMPLING_FACTOR_FROM_MAKEFILE
#define OVERSAMPLING_RATE  (SAMPLING_RATE*OVERSAMPLING_FACTOR)

/* 
 * FFT frame lengths (number of samples in a frame). 
 * WARNING only FRAME_LEN_4096 is currently supported.
 */
enum frame_length {
	FRAME_LEN_32   = 32,
	FRAME_LEN_64   = 64,
	FRAME_LEN_128  = 128,
	FRAME_LEN_256  = 256,
	FRAME_LEN_512  = 512,
	FRAME_LEN_1024 = 1024,
	FRAME_LEN_2048 = 2048,
	FRAME_LEN_4096 = 4096
};
/* 
 * Frame related buffers are generally of this length because all lesser frame 
 * lengths will fit in it. 
 */
#define MAX_FRAME_LEN  FRAME_LEN_4096
#define MAX_NR_BINS (MAX_FRAME_LEN/2)


/*
 * Transform an oversized frame of audio samples in the time domain to and return the 
 * frequency bin magnitudes (energy) of its spectra in the frequency domain. 
 *
 * The oversized frame of samples is of length frame_len*OVERSAMPLING_FACTOR, and it is 
 * assumed the samples were captured at an oversampling rate of OVERSAMPLING_RATE.
 * Prior to FFT the oversized frame is low-pass filtered and then decimated down to the
 * original SAMPLING_RATE and frame_len. See comment at SAMPLING_RATE for why oversampling
 * is done at all. 
 *
 * The frequency bandwidth (frequencies from 0 to half the SAMPLING_RATE)
 * are split across nr_bins() frequency bins (hence the returned array is also
 * of length nr_bins()). The frequency range of bin at index i spans bin_width() Hz
 * and is centred about frequency i*bin_width() (see also bin_index_to_freq()). 
 * Bins start at index 1 because index 0 is for the DC offset and can be ignored.
 *
 * WARNING
 * - The input samples array is reused and trashed by the implementation of this function in
 *   order to save MCU RAM space.
 * - The return is a static buffer and will trash the previous return when
 *   called in sequence.
 */
void samples_to_freq_bin_magnitudes_init(enum frame_length frame_len);
float32_t *samples_to_freq_bin_magnitudes(float32_t *samples, enum frame_length frame_len);

int nr_bins(enum frame_length frame_len);
int bandwidth(int sampling_rate);
/*
 * The larger the frame length is relative to the sampling rate, the smaller the bin width, and so 
 * the better the frequency resolution. But this comes at the cost of time resolution, as now the 
 * frame takes longer to fill.
 */
float32_t bin_width(enum frame_length frame_len, int sampling_rate);
/* Get the index of the bin which the frequency falls into. */
int freq_to_bin_index(float32_t frequency, float32_t binwidth);
float32_t bin_index_to_freq(int bin_index, float32_t binwidth);
int nyquist_frequency(int sampling_rate);

/* Number of harmonics can't be too high otherwise the new product value will overflow. */
#define NHARMONICS 4

/*
 * Apply a Harmonic Product Spectrum (HPS) to the magnitudes to turn the fundamental
 * frequency peak into the maximum peak. This is done because the maximum peak isn't 
 * necessarily the fundamental, and may be a different harmonic.
 */
void harmonic_product_spectrum(float32_t *freq_bin_magnitudes, enum frame_length frame_len,
			       int sampling_rate);
/* Get the index of the frequency bin with the maximum magnitude peak. */
int max_bin_index(float32_t *freq_bin_magnitudes, enum frame_length frame_len);

#endif
