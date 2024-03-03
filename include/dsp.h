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
 * to meet a reasonable frequency/time resolution tradeoff when paired with
 * the MAX_FRAME_LEN.
 * 
 * See also comments at 'note.c:note_freqs'.
 * TODO reword ending and ref frequency/time resolution stuff when implemented elsewhere
 */
#define OVERSAMPLING_RATE 4096

/* Supported FFT frame lengths (number of samples in a frame). */
enum frame_length {
/* TODO rm unused later on*/
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
 * aliasing (see core/gen_filter_coeffs.m and OVERSAMPLING_RATE for more info). 
 *
 * TODO 
 * - as write up more source and things get documented elsewhere, add more here or
 *   move documentation it away wherever it's best suited. and use refs to defines
 *   where possible (as they too get added)
 * - explain why takes s16 integers?
 * - explain resolution tradeoffs here or elsewhere? (probably elsewhere because this
 *   is getting verbose)
 * - handle errors and return NULL on error?
 */
float32_t *samples_to_freq_bin_magnitudes(const int16_t *samples, enum frame_length frame_len);

int nr_bins(enum frame_length frame_len);
int bandwidth(void);
int bin_width(enum frame_length frame_len);
enum frame_length frame_length_from_bin_width(int binwidth);  /* Return 0 on error. */
/* Get the index of the bin which the frequency falls into. */
int freq_to_bin_index(float32_t frequency, int binwidth);
float32_t bin_index_to_freq(int bin_index, int binwidth);

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
