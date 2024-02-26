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
static void plot_magnitudes(FILE *gnuplot, const int16_t *samples, enum frame_length frame_len)
{
	float32_t *freq_bin_magnitudes = guitar_tuner_dsp(samples, frame_len);
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
 * Save a plot of the output of the note samples after processing with guitar_tuner_dsp() 
 * to file 'plot/<note_name>-<i>.svg'.
 */
static bool plot_note_freq_bin_magnitudes(const char *note_name, int i, 
					  const int16_t *samples, enum frame_length frame_len) 
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
	snprintf(plot_pathname, sizeof(plot_pathname), "plot/%s-%d.%s", note_name, i, image_file_format);
	fprintf(gnuplot, "set output '%s'\n", plot_pathname);

	fprintf(gnuplot, "set title '%s'\n", note_name);
	fprintf(gnuplot, "set xlabel 'Frequency (Hz)'\n");
	fprintf(gnuplot, "set ylabel 'Magnitude'\n"); /* TODO squared? or energy? */
	fprintf(gnuplot, "set xtics out nomirror %d\n", XTICS_INCR);
	fprintf(gnuplot, "set ytics out nomirror\n");
	fprintf(gnuplot, "set style fill solid\n");

	plot_magnitudes(gnuplot, samples, frame_len);

	pclose(gnuplot);
	printf("Wrote plot to %s\n", plot_pathname);
	return true;
}

int main(void)
{
	/* TODO test with different frame lengths */
	/* TODO add cmdline opt for frame length? */
	return !for_each_note_file_source(FRAME_LEN_4096, plot_note_freq_bin_magnitudes);
}

