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

/*
 * Transform a frame of audio samples in the time domain to and return the 
 * frequency bin magnitudes (energy) of its spectra in the frequency domain. 
 * Note the return is a static buffer and will trash the previous return when
 * called in sequence.
 *
 * The frequency bandwidth (frequencies from 0 to half the OVERSAMPLNG_RATE)
 * are split across nr_bins() frequency bins (hence the returned array is also
 * of length nr_bins()). Each bin spans bin_width() Hz: the bin at index i 
 * stores the magnitude for the range of frequencies between (i-1)*bin_width Hz 
 * and i*bin_width Hz. Bins start at index 1 because index 0 is for the DC offset 
 * and can be ignored.
 * TODO confirm?
 *
 * Note a band-pass filter is also applied, its low-pass (anti-aliasing) filter part 
 * done specifically to cut off frequencies above the Nyquist frequency and prevent
 * aliasing (see core/gen_filter_coeffs.m and OVERSAMPLING_RATE for more info). 
 *
 * TODO 
 * - as write up more source and things get documented elsewhere, add more here or
 *   move documentation it away wherever it's best suited. and use refs to defines
 *   where possible (as they too get added)
 * - rename?
 * - explain why takes s16 integers?
 * - explain resolution tradeoffs here or elsewhere? (probably elsewhere because this
 *   is getting verbose)
 * - handle errors and return NULL on error?
 */
float32_t *guitar_tuner_dsp(const int16_t *samples, enum frame_length frame_len);

int nr_bins(enum frame_length frame_len);
int bandwidth(void);
int bin_width(enum frame_length frame_len);

#endif
