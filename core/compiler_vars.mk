# Set (implicit) compiler variables for implicit rules required to build 
# objects for the core library, and objects dependent on it.
#
# Ensure variable arm_arch_profile is set before including (see below error for
# its possible values), and optionally cross_prefix to cross compile.

ifndef arm_arch_profile
$(error Variable arm_arch_profile not defined: set it to either 'A' for Cortex-A or 'M' for Cortex-M)
endif
# TODO if cortex-m then assert cross prefix required? if do this then fix up end of 
# core/Makefile topmost comment?

export CC = $(cross_prefix)gcc
export CFLAGS = -iquote ../include -I../CMSIS-DSP/Include -I../CMSIS_6/CMSIS/Core/Include \
		-D__ARM_ARCH_PROFILE=$(arm_arch_profile)

# TODO 
# - target a processor / arch in CFLAGS? maybe only for mcu, and for tests
# can mention to use qemu with a cortex-a cpu if not running natively on a
# cortex-a pi/machine
# - __ARM_ARCH_PROFILE gets defined automatically when you target a cpu?
# - test compiling natively (not cross compiling) on rpi4
