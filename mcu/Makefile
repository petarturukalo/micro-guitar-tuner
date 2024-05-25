# TODO 
# - add prereq for libopencm3 that does a git submodule init. 
#   also add the same for the cmsis libs in libcore
# - explain somewhere that this is for the STM32 blackpill with F4? (be more precise?)
export cross_prefix = arm-none-eabi-
export arm_arch_profile = M
cpu = cortex-m4
include ../core/compiler_vars.mk
CFLAGS += -Ilibopencm3/include -DSTM32F4 -mthumb
# Use the Cortex-M4 FPU, which implements the FPv4-SP floating point extension, for (TODO very?) fast 
# single-precision float calculations. TODO does it end up being fast? TODO needed by processing core (explain why)
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16  
LDFLAGS = -Wl,-T,stm32f411ceu6.ld -nostartfiles
# Prompt gcc to link against the hard float libc instead of the default libc.
LDFLAGS += -march=armv7e-m+fp -mfloat-abi=hard
# Link against libc for printf(). Libc provides the interface for syscalls like 
# write() (used by printf()), but its implementation is provided by the OS (Linux), 
# which isn't available here, so link against nosys to provide stub implementations 
# for the functions we don't care about but are needed to compile.
LDLIBS = -lc -lnosys -lm

objs = guitar_tuner.o ssd1306.o font.o debug.o
libcore = ../core/libcore-M.a
libopencm3 = libopencm3/lib/libopencm3_stm32f4.a


guitar-tuner.bin: guitar-tuner.elf
	$(cross_prefix)objcopy -O binary $< $@

guitar-tuner.elf: $(objs) $(libcore) $(libopencm3)
	$(CC) $(LDFLAGS) -o $@  $^ $(LDLIBS)
	$(cross_prefix)size $@

# Includes libopencm3/stm32/f4/nvic.h which doesn't exist until libopencm3 is built.
guitar_tuner.c: $(libopencm3)

$(libcore):
	$(MAKE) -C ../core

$(libopencm3): 
	$(MAKE) -C libopencm3 TARGETS=stm32/f4

flash:
	st-flash --flash=512k write guitar-tuner.bin 0x8000000

# TODO don't bother cleaning libopencm3? also mention why not cleaning it?
clean:
	-rm guitar-tuner.bin guitar-tuner.elf $(objs)
	-$(MAKE) -C ../core clean

