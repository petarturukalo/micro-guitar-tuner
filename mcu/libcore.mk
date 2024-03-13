# Build the Cortex-M version of libcore.

export cross_prefix = arm-none-eabi-
export arm_arch_profile = M
cpu = cortex-m0plus
include ../core/compiler_vars.mk

libcore = ../core/libcore-M.a

$(libcore):
	$(MAKE) -C ../core

# TODO need this clean to be called from cmake?
clean: 
	-$(MAKE) -C ../core clean

