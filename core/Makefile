# This makefile shouldn't be used directly, only invoked from a parent makefile 
# which has a dependency on this core lib. To use it from such a parent makefile,
# ensure to include compiler_vars.mk (see it for more info) and, if cross compiling, 
# that the cross_prefix variable (mentioned in compiler_vars.mk) is exported.

# TODO using ARM_ALL_FFT_TABLES for now during testing. but replace it with only the
# defines for the tables i need (see RFFT_FAST_F32_<frame len> from CMSIS-DSP/Source/fft.cmake,
# and explanation in CMSIS-DSP/README.md). also explain what the defines are for after replacement
CFLAGS += -DARM_FFT_ALLOW_TABLES \
	  -DARM_ALL_FFT_TABLES

# Objects local to the core lib.
objs = dsp.o
# Dependent CMSIS DSP objects.
# TODO better way to do this? maybe CMSIS DSP provides a cmake interface?
objs += ../CMSIS-DSP/Source/FilteringFunctions/arm_fir_init_f32.o \
	../CMSIS-DSP/Source/FilteringFunctions/arm_fir_f32.o \
	../CMSIS-DSP/Source/TransformFunctions/arm_rfft_fast_init_f32.o \
	../CMSIS-DSP/Source/TransformFunctions/arm_rfft_fast_f32.o \
	../CMSIS-DSP/Source/TransformFunctions/arm_cfft_init_f32.o \
	../CMSIS-DSP/Source/TransformFunctions/arm_cfft_f32.o \
	../CMSIS-DSP/Source/CommonTables/arm_common_tables.o \
	../CMSIS-DSP/Source/CommonTables/arm_const_structs.o \
	../CMSIS-DSP/Source/TransformFunctions/arm_cfft_radix8_f32.o \
	../CMSIS-DSP/Source/TransformFunctions/arm_bitreversal2.o \
	../CMSIS-DSP/Source/ComplexMathFunctions/arm_cmplx_mag_f32.o


libcore.a: $(objs)
	$(cross_prefix)$(AR) rsc --thin $@ $^

clean:
	rm $(objs) libcore.a

# TODO
# - when start on mcu code and compiling for cortex-m, need to make 
# sure cortex-m objs don't get mixed together with cortex-a objs. 
# might need to give them different names, and the libs diff names too.
