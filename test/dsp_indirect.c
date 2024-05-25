#include "dsp_indirect.h"

static void s16_array_to_f32(const int16_t *src, float32_t *dest, int len)
{
	for (int i = 0; i < len; ++i) 
		dest[i] = (float32_t)src[i];
}

float32_t *samples_to_freq_bin_magnitudes_s16(const int16_t *samples, enum frame_length frame_len)
{
	static float32_t float_samples[MAX_FRAME_LEN];
	s16_array_to_f32(samples, float_samples, frame_len);
	return samples_to_freq_bin_magnitudes(float_samples, frame_len);
}

