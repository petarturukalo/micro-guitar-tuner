# Copyright (C) 2024 Petar Turukalo
# SPDX-License-Identifier: GPL-2.0
#
# Set (implicit) compiler variables for (implicit) rules required to build 
# objects for the core library, and objects dependent on it.
#
# This makefile asserts the following variables are set.
# - arm_arch_profile: A for Cortex-A or M for Cortex-M
# - cross_prefix: cross compilation prefix. Required for Cortex-M. 
# - cpu: processor to target. Only for Cortex-M, and required for it.

arm_arch_profile_error_msg = "Variable arm_arch_profile not defined or set correctly: set it to either A for Cortex-A or M for Cortex-M"
ifndef arm_arch_profile
$(error $(arm_arch_profile_error_msg))
endif
ifneq ($(arm_arch_profile), A)
ifneq ($(arm_arch_profile), M)
$(error $(arm_arch_profile_error_msg))
endif
endif

ifeq ($(arm_arch_profile), M)
ifndef cross_prefix
$(error Cortex-M needs variable cross_prefix defined)
endif
ifndef cpu
$(error Cortex-M needs variable cpu defined)
endif
endif

include ../core/dsp_params.mk

export CC = $(cross_prefix)gcc
export CFLAGS = -iquote ../include -I../CMSIS-DSP/Include -I../CMSIS_6/CMSIS/Core/Include \
		-DSAMPLING_RATE_FROM_MAKEFILE=$(sampling_rate) \
		-DOVERSAMPLING_FACTOR_FROM_MAKEFILE=$(oversampling_factor)
# Only explicitly define __ARM_ARCH_PROFILE for Cortex-A because Cortex-M has it
# implicitly defined through its -mcpu option, and we don't want to redefine it.
ifneq ($(arm_arch_profile), M)
CFLAGS += -D__ARM_ARCH_PROFILE="'$(arm_arch_profile)'"
else
CFLAGS += -mcpu=$(cpu)
endif

