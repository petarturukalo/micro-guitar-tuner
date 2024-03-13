# This makefile shouldn't be used directly, only invoked from a parent makefile 
# which has a dependency on this core lib. To use it from such a parent makefile,
# ensure to include compiler_vars.mk (see it for more info) and, if cross compiling, 
# that the cross_prefix variable (mentioned in compiler_vars.mk) is exported.

# Allow the FFT table defines to take effect.
CFLAGS += -DARM_DSP_CONFIG_TABLES -DARM_FFT_ALLOW_TABLES
# Flags to enable 32-bit float real FFT, frame length 4096. 
# See RFFT_FAST_<type>_<frame len> (e.g. RFFT_FAST_F32_4096) from CMSIS-DSP/Source/fft.cmake 
# for the defines needed to use a real FFT of a particular data type and frame length.
# Only define exactly what's needed as each real FFT type uses its own tables which
# take up a lot of memory and will bloat the final executable.
# TODO enable frame len 2048?
CFLAGS += -DARM_TABLE_TWIDDLECOEF_F32_2048 -DARM_TABLE_BITREVIDX_FLT_2048 -DARM_TABLE_TWIDDLECOEF_RFFT_F32_4096
# TODO 
CFLAGS += -Ofast

# Objects local to the core lib.
objs = dsp.o note.o adc.o
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
	../CMSIS-DSP/Source/ComplexMathFunctions/arm_cmplx_mag_f32.o \
	../CMSIS-DSP/Source/StatisticsFunctions/arm_max_f32.o \
	../CMSIS-DSP/Source/FastMathFunctions/arm_vlog_f32.o


libcore.a: $(objs)
	$(cross_prefix)$(AR) rsc --thin $@ $^

clean:
	rm $(objs) libcore.a

# TODO
# - when start on mcu code and compiling for cortex-m, need to make 
# sure cortex-m objs don't get mixed together with cortex-a objs. 
# might need to give them different names, and the libs diff names too
# (suffix with -a and -m before the extension??).
