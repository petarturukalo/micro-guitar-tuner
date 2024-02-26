export cross_prefix = arm-linux-gnueabihf-
arm_arch_profile = "'A'"
include ../core/compiler_vars.mk
# Fix for readdir() not working when emulating 32-bit ARM binary.
CFLAGS += -D_FILE_OFFSET_BITS=64

gen_plots_bin = gen-freq-mag-plots
gen_plot_objs = plot.o file_source.o
libcore = ../core/libcore.a


$(gen_plots_bin): $(gen_plot_objs) $(libcore)
	$(CC) $^ -o $@

$(libcore):
	$(MAKE) -C ../core

clean: 
	-rm $(gen_plot_objs) $(gen_plots_bin) plot/*.svg
	-$(MAKE) -C ../core clean

