/*
 * Copyright (C) 2024 Petar Turukalo
 * SPDX-License-Identifier: GPL-2.0
 */
#ifndef DSP_H
#define DSP_H

#include <stdint.h>
#include <arm_math_types.h>

/*
 * The actual sampling rate, after oversampling. The "original" sampling rate
 * is defined as twice the highest frequency which I intend to handle in this
 * tuner (Nyquist theorem). That cutoff frequency (Nyquist frequency) is defined 
 * at core/gen_filter_coeffs.m:lowpass_cutoff_freq as 1700 Hz, which twice of 
 * gives a 3400 Hz sampling rate. The actual sampling rate is a bit over this
 * (hence oversampling), to 1. reduce quantization noise / improve SNR, and 2. 
 * to meet a reasonable frequency/time resolution tradeoff (see explanation at bin_width()) 
 * when paired with the frame length.
 * 
 * See also comments at '../core/note.c:note_freqs'.
 */
#define OVERSAMPLING_RATE  OVERSAMPLING_RATE_FROM_MAKEFILE

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
 * Transform a frame of audio samples in the time domain to and return the 
 * frequency bin magnitudes (energy) of its spectra in the frequency domain. 
 * Note the return is a static buffer and will trash the previous return when
 * called in sequence.
 *
 * The frequency bandwidth (frequencies from 0 to half the OVERSAMPLNG_RATE)
 * are split across nr_bins() frequency bins (hence the returned array is also
 * of length nr_bins()). The frequency range of bin at index i spans bin_width() Hz
 * and is centred about frequency i*bin_width() (see also bin_index_to_freq()). 
 * Bins start at index 1 because index 0 is for the DC offset and can be ignored.
 *
 * Note a band-pass filter is also applied, its low-pass (anti-aliasing) filter part 
 * done specifically to cut off frequencies above the Nyquist frequency and prevent
 * aliasing (see ../core/gen_filter_coeffs.m and OVERSAMPLING_RATE for more info). 
 */
void samples_to_freq_bin_magnitudes_init(enum frame_length frame_len);
float32_t *samples_to_freq_bin_magnitudes(const float32_t *samples, enum frame_length frame_len);

int nr_bins(enum frame_length frame_len);
int bandwidth(void);
/*
 * The bin width is dependent on the sampling rate and frame length: the larger the frame length
 * is relative to the sampling rate, the smaller the bin width, and so the better the frequency 
 * resolution. But this comes at the cost of time resolution, as now the frame takes longer to fill.
 *
 * Note this function assumes the sampling rate is OVERSAMPLING_RATE.
 */
float32_t bin_width(enum frame_length frame_len);
/* Get the index of the bin which the frequency falls into. */
int freq_to_bin_index(float32_t frequency, float32_t binwidth);
float32_t bin_index_to_freq(int bin_index, float32_t binwidth);

/* Number of harmonics can't be too high otherwise the new product value will overflow. */
#define NHARMONICS 4

/*
 * Apply a Harmonic Product Spectrum (HPS) to the magnitudes to turn the fundamental
 * frequency peak into the maximum peak. This is done because the maximum peak isn't 
 * necessarily the fundamental, and may be a different harmonic.
 */
void harmonic_product_spectrum(float32_t *freq_bin_magnitudes, enum frame_length frame_len);
/* Get the index of the frequency bin with the maximum magnitude peak. */
int max_bin_index(float32_t *freq_bin_magnitudes, enum frame_length frame_len);

#endif
