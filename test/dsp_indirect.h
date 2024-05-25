/*
 * Some test sources include this to indirectly include dsp.h from 
 * the core library as they require the signed 16-bit integer version 
 * of samples_to_freq_bin_magnitudes().
 *
 * Function samples_to_freq_bin_magnitudes_s16() is not defined in the 
 * core library to not waste MCU RAM space.
 */
#ifndef DSP_INDIRECT
#define DSP_INDIRECT

#include "dsp.h"

float32_t *samples_to_freq_bin_magnitudes_s16(const int16_t *samples, enum frame_length frame_len);

#endif
