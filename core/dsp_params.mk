# Copyright (C) 2024 Petar Turukalo
# SPDX-License-Identifier: GPL-2.0

# Number of filter coefficients. See the gen_filter_coeffs.m script for more info.
# The filtering can be a huge bottleneck for the processing, especially on a MCU
# without a FPU such as the pico, so this can't be too high. 
export nr_taps = 128
# Defined in make so it can be shared between C and octave sources.
# See comment at ../include/dsp.h:OVERSAMPLING_RATE
export oversampling_rate = 4000

