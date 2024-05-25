/*
 * Copyright (C) 2024 Petar Turukalo
 * SPDX-License-Identifier: GPL-2.0
 */
#ifndef ADC_H
#define ADC_H

#include <stdint.h>
#include <arm_math_types.h>

/*
 * The MCU ADC read function returns the voltage of the electrical
 * signal in range 0 to 3.3V encoded in a 12-bit ("unsigned") value. 
 * Convert this value to a 32-bit float storing a value in the range
 * of a signed 16-bit integer, the reason for this format being that
 * the test data is signed 16-bit but converted to 32-bit float for
 * DSP anyway, and to not lose resolution when uspcaling to signed 16-bit.
 */
float32_t convert_adc_u12_sample_to_s16(uint16_t u12_sample);

#endif
