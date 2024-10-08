# Copyright (C) 2024 Petar Turukalo
# SPDX-License-Identifier: GPL-2.0
#
# This makefile shouldn't be used directly, only invoked from a parent makefile 
# which has a dependency on this core lib. To use it from such a parent makefile,
# ensure to include compiler_vars.mk (see it for more info) and that variables 
# arm_arch_profile and cross_prefix are exported (the latter only if cross compiling).
#
# Users of this lib must also link against libm.

include dsp_params.mk

CFLAGS += -DNR_TAPS=$(nr_taps) 
# Flags to enable 32-bit float real FFT, frame length 4096. 
# See RFFT_FAST_<type>_<frame len> (e.g. RFFT_FAST_F32_4096) from CMSIS-DSP/Source/fft.cmake 
# for the defines needed to use a real FFT of a particular data type and frame length.
# Only define exactly what's needed as each real FFT type uses its own tables which
# take up a lot of memory and will bloat the final executable.
CFLAGS += -DARM_DSP_CONFIG_TABLES -DARM_FFT_ALLOW_TABLES \
	  -DARM_TABLE_TWIDDLECOEF_F32_2048 -DARM_TABLE_BITREVIDX_FLT_2048 -DARM_TABLE_TWIDDLECOEF_RFFT_F32_4096
# Recommended by CMSIS DSP for best performance (and from testing it does improve processing time considerably).
CFLAGS += -Ofast

# Objects local to the core lib.
objs = dsp.o note.o adc.o filter_coeffs.o 2d_bit_array.o
# Dependent CMSIS DSP objects.
objs += ../CMSIS-DSP/Source/FilteringFunctions/arm_fir_decimate_init_f32.o \
	../CMSIS-DSP/Source/FilteringFunctions/arm_fir_decimate_f32.o \
	../CMSIS-DSP/Source/TransformFunctions/arm_rfft_fast_init_f32.o \
	../CMSIS-DSP/Source/TransformFunctions/arm_rfft_fast_f32.o \
	../CMSIS-DSP/Source/TransformFunctions/arm_cfft_init_f32.o \
	../CMSIS-DSP/Source/TransformFunctions/arm_cfft_f32.o \
	../CMSIS-DSP/Source/CommonTables/arm_common_tables.o \
	../CMSIS-DSP/Source/CommonTables/arm_const_structs.o \
	../CMSIS-DSP/Source/TransformFunctions/arm_cfft_radix8_f32.o \
	../CMSIS-DSP/Source/TransformFunctions/arm_bitreversal2.o \
	../CMSIS-DSP/Source/ComplexMathFunctions/arm_cmplx_mag_f32.o \
	../CMSIS-DSP/Source/StatisticsFunctions/arm_max_f32.o 
# Suffix objects and lib with ARM arch profile to separate the test (Cortex-A) and MCU 
# (Cortex-M) build artifacts and prevent them from clashing and breaking the other's build.
objs := $(patsubst %.o,%-$(arm_arch_profile).o,$(objs))
libcore = libcore-$(arm_arch_profile).a


.NOTPARALLEL:

$(libcore): ../CMSIS-DSP/CMakeLists.txt $(objs) 
	$(cross_prefix)$(AR) rscT $@ $^

%-$(arm_arch_profile).o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

filter_coeffs.c: print_filter_coeffs.m gen_filter_coeffs.m
	octave print_filter_coeffs.m $(oversampling_rate) $(nr_taps) $(oversampling_factor) > $@

../CMSIS-DSP/CMakeLists.txt:
	git submodule update --init

clean:
	rm $(objs) filter_coeffs.c $(libcore)

plot-filter-coeffs: plot_filter_coeffs.m gen_filter_coeffs.m
	octave plot_filter_coeffs.m $(oversampling_rate) $(nr_taps) $(oversampling_factor)

