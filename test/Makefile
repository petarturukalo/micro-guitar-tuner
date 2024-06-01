# Copyright (C) 2024 Petar Turukalo
# SPDX-License-Identifier: GPL-2.0

export cross_prefix = arm-linux-gnueabihf-
# Although the MCU is Cortex-M, emulate Cortex-A instead because these 
# are user space applications which test non-MCU specific things, e.g. 
# CMSIS DSP which supports both Cortex-A and Cortex-M. 
export arm_arch_profile = A
include ../core/compiler_vars.mk
# Fix for readdir() not working when emulating 32-bit ARM binary.
CFLAGS += -D_FILE_OFFSET_BITS=64

gen_plots_bin = gen-freq-mag-plots
assert_tests_bin = assert-tests
gen_plot_objs = plot.o file_source.o dsp_indirect.o
assert_tests_objs = assert_tests.o assert.o file_source.o dsp_indirect.o
libcore = ../core/libcore-A.a


.NOTPARALLEL:

all: $(gen_plots_bin) $(assert_tests_bin)

$(gen_plots_bin): $(libcore) $(gen_plot_objs) 
	$(CC) -o $@  $(gen_plot_objs) $(libcore) -lm

$(assert_tests_bin): $(libcore) $(assert_tests_objs) 
	$(CC) -o $@  $(assert_tests_objs) $(libcore) -lm

$(libcore):
	$(MAKE) -C ../core 

clean: 
	-rm $(gen_plot_objs) $(gen_plots_bin) plot/*.svg
	-rm $(assert_tests_objs) $(assert_tests_bin) 
	-$(MAKE) -C ../core clean

