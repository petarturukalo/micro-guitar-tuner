/*
 * Copyright (C) 2024 Petar Turukalo
 * SPDX-License-Identifier: GPL-2.0
 */
#include <stdint.h>
#include "adc.h"

#define ADC_UINT12_MAX 4095
#define ADC_UINT12_ZERO_VAL 2047.5
#define ADC_UINT12_MIN_NEG -2047.5
#define ADC_UINT12_MAX_POS  2047.5

float32_t convert_adc_u12_sample_to_s16(uint16_t u12_sample)
{
	float32_t diff_from_zero, s16_sample;
	static const float32_t scale_to_neg_int16 = INT16_MIN/ADC_UINT12_MIN_NEG;
	static const float32_t scale_to_pos_int16 = INT16_MAX/ADC_UINT12_MAX_POS;
	
	/* 
	 * The microphone has a DC bias of VCC/2 = 3.3/2 = 1.65V, i.e. the 
	 * "zero" value is at 1.65V: all readings below 1.65V are negative 
	 * numbers, and all above are positive. The 12-bit encoded value
	 * of 3.3V is ADC_UINT12_MAX, so the 12-bit encoded value of 1.65V is
	 * half ADC_UINT12_MAX (or ADC_UINT12_ZERO_VAL).
	 */
	if (u12_sample < ADC_UINT12_ZERO_VAL) {
		/* Sample is a negative number. */
		diff_from_zero = -(ADC_UINT12_ZERO_VAL-u12_sample);
		s16_sample = diff_from_zero*scale_to_neg_int16;
	} else {
		/* Sample is a positive number. */
		diff_from_zero = u12_sample-ADC_UINT12_ZERO_VAL;
		s16_sample = diff_from_zero*scale_to_pos_int16;
	}
	return s16_sample;
}
