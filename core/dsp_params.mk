# Copyright (C) 2024 Petar Turukalo
# SPDX-License-Identifier: GPL-2.0
# 
# These varibles are defined in make so they can be shared between C and octave sources.

# Number of filter coefficients. See the gen_filter_coeffs.m script for more info.
# The filtering can be a huge bottleneck for the processing, especially on a MCU
# without a FPU such as the pico, so this can't be too high. 
export nr_taps = 128
# See comment at ../include/dsp.h:SAMPLING_RATE
export sampling_rate = 4000
# The higher the oversampling factor, the more higher frequencies that can be prevented 
# from aliasing, but it comes at the cost of RAM and performance, and so this can't
# be too high. See also comment at ../include/dsp.h:OVERSAMPLING_FACTOR
export oversampling_factor = 2
export oversampling_rate = $(shell expr $(sampling_rate) \* $(oversampling_factor))

