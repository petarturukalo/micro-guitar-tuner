#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "file_source.h"
#include "dsp.h"

#define XTICS_INCR 100

/* 
 * Plot magnitudes, drawing a box of width bin_width() for each. 
 * Metadata keyentries are also drawn to the key/legend here because it can only
 * be done with the plot command.
 */
static void plot_magnitudes(FILE *gnuplot, float32_t *freq_bin_magnitudes, enum frame_length frame_len)
{
	const int nbins = nr_bins(frame_len);
	const int binwidth = bin_width(frame_len);
	float bin_centre_xpos = binwidth/2;

	fprintf(gnuplot, "set key noautotitle\n");  /* Remove default keyentry. */
	fprintf(gnuplot, "set key inside right top offset -5,0\n");

	fprintf(gnuplot, "plot '-' with boxes, ");
	/* Add keyentries for metadata. */
	fprintf(gnuplot, "keyentry 'frame len %d', ", frame_len);
	fprintf(gnuplot, "keyentry 'nbins %d', ", nbins);
	fprintf(gnuplot, "keyentry 'bin width %d Hz'\n", binwidth);
	/* Plot magnitudes. Start at 1 to skip DC. */
	for (int i = 1; i < nbins; ++i) {
		fprintf(gnuplot, "%.1f %a\n", bin_centre_xpos, freq_bin_magnitudes[i]);
		bin_centre_xpos += binwidth;
	}
	fprintf(gnuplot, "e\n");
}

/*
 * Save a plot of the frequency bin magnitudes of a note to file 'plot/<note_name><name_suffix>-<i>.svg'.
 */
static bool _plot_note_freq_bin_magnitudes(const char *note_name, char *name_suffix, int i, 
					   float32_t *freq_bin_magnitudes, enum frame_length frame_len) 
{
	FILE *gnuplot;
	const char *image_file_format = "svg";
	char plot_pathname[64];

	gnuplot = popen("gnuplot", "w");
	if (!gnuplot) {
		fprintf(stderr, "Error opening gnuplot: %s\n", strerror(errno));
		return false;
	}
	fprintf(gnuplot, "set term %s size 1024,768 dynamic mouse\n", image_file_format); 
	snprintf(plot_pathname, sizeof(plot_pathname), "plot/%s%s-%d.%s", note_name, name_suffix, i, image_file_format);
	fprintf(gnuplot, "set output '%s'\n", plot_pathname);

	fprintf(gnuplot, "set title '%s'\n", note_name);
	fprintf(gnuplot, "set xlabel 'Frequency (Hz)'\n");
	fprintf(gnuplot, "set ylabel 'Magnitude'\n"); 
	fprintf(gnuplot, "set xtics out nomirror %d\n", XTICS_INCR);
	fprintf(gnuplot, "set ytics out nomirror\n");
	fprintf(gnuplot, "set style fill solid\n");

	plot_magnitudes(gnuplot, freq_bin_magnitudes, frame_len);

	pclose(gnuplot);
	printf("Wrote plot to %s\n", plot_pathname);
	return true;
}

static bool plot_note_freq_bin_magnitudes(const char *note_name, int i, 
					  const int16_t *samples, enum frame_length frame_len) 
{
	float32_t *freq_bin_magnitudes = samples_to_freq_bin_magnitudes_s16(samples, frame_len);
	return _plot_note_freq_bin_magnitudes(note_name, "", i, freq_bin_magnitudes, frame_len);
}

static bool plot_note_freq_bin_magnitudes_hps(const char *note_name, int i, 
					      const int16_t *samples, enum frame_length frame_len) 
{
	float32_t *freq_bin_magnitudes = samples_to_freq_bin_magnitudes_s16(samples, frame_len);
	harmonic_product_spectrum(freq_bin_magnitudes, frame_len);
	return _plot_note_freq_bin_magnitudes(note_name, "-hps", i, freq_bin_magnitudes, frame_len);
}

/*
 * TODO
 * - test with different frame lengths
 * - add cmdline opt for frame length? 
 * - even add cmdline opts for whether to produce mag plots, hps plots, all i frames 
 *	or just i==1 frame (to save space)
 */
int main(void)
{
	return !for_each_note_file_source(FRAME_LEN_4096, plot_note_freq_bin_magnitudes) ||
	       !for_each_note_file_source(FRAME_LEN_4096, plot_note_freq_bin_magnitudes_hps);
}

