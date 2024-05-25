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


all: $(gen_plots_bin) $(assert_tests_bin)

# TODO -lm only here because logf needed by arm_vlog_f32.
# make sure it's statically linked and if end up needing to
# link against math lib then mention it somewhere in core makefile?
#
# if changes to use q notation rather than f32 then might not need
# logf anymore
# TODO put input files $^ at separated by two spaces (for all bins)
$(gen_plots_bin): $(gen_plot_objs) $(libcore)
	$(CC) -o $@  $^ -lm

# TODO ensure -lm linked static?
$(assert_tests_bin): $(assert_tests_objs) $(libcore)
	$(CC) -o $@  $^ -lm

$(libcore):
	$(MAKE) -C ../core 

clean: 
	-rm $(gen_plot_objs) $(gen_plots_bin) plot/*.svg
	-rm $(assert_tests_objs) $(assert_tests_bin) 
	-$(MAKE) -C ../core clean

