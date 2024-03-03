#include "dsp.h"
#include "note.h"
#include "assert.h"
#include "file_source.h"
#include <stddef.h>
#include <stdio.h>
#include <math.h>

/* Assert the frequency of a sine wave falls into the expected bin. */
/* TODO note 83 Hz is failing bin width 2 for some reason. not 100% certain on rounding */
static bool assert_sine_wave_freq_to_bin_index(const char *sine_freq_str, int i, const int16_t *samples, enum frame_length frame_len)
{
	float32_t sine_freq;
	float32_t *freq_bin_magnitudes;
	int expected_bin_index, actual_bin_index;

	sscanf(sine_freq_str, "%f", &sine_freq);
	freq_bin_magnitudes = samples_to_freq_bin_magnitudes(samples, frame_len);
	expected_bin_index = freq_to_bin_index(sine_freq, bin_width(frame_len));
	actual_bin_index = max_bin_index(freq_bin_magnitudes, frame_len);

	Assert(expected_bin_index == actual_bin_index, "expected sine %.2f Hz at bin index %d but was %d, bin width %d", 
							sine_freq, expected_bin_index, actual_bin_index, bin_width(frame_len));
	return true;
}

/*
 * Generate peaks in the empty magnitudes array at the first NHARMONICS harmonics of the fundamental
 * frequency. Return the expected magnitude value of the fundamental frequency / maximum peak after HPS.
 */
static float32_t generate_harmonic_peaks(float32_t fundamental_freq, float32_t *mags, int binwidth)
{
	const int harmonic_peak_mag = 2;  /* At least higher than 1 so a multiplication with it returns an increased value. */
	const int fundamental_freq_bin_index = freq_to_bin_index(fundamental_freq, binwidth);

	mags[fundamental_freq_bin_index] = harmonic_peak_mag;
	for (int harmonic = 2; harmonic <= NHARMONICS; ++harmonic) {
		int harmonic_bin_index = harmonic*fundamental_freq_bin_index;
		mags[harmonic_bin_index] = harmonic_peak_mag;
	}
	return pow(harmonic_peak_mag, NHARMONICS);
}

/* Assert the implementation of harmonic_product_spectrum() finds the peak value of harmonics as intended. */
static void assert_hps_find_harmonic_peaks(float32_t fundamental_freq, int binwidth)
{
	float32_t mags[MAX_NR_BINS] = { 0 };
	int expected_bin_index, actual_bin_index;
	float32_t expected_mag, actual_mag;
	enum frame_length frame_len;

	frame_len = frame_length_from_bin_width(binwidth);
	Assert(frame_len, "no frame length supported for %d bin width", binwidth);

	expected_bin_index = freq_to_bin_index(fundamental_freq, binwidth);
	expected_mag = generate_harmonic_peaks(fundamental_freq, mags, binwidth);
	harmonic_product_spectrum(mags, frame_len);
	actual_bin_index = max_bin_index(mags, frame_len);
	actual_mag = mags[actual_bin_index];
	
	Assert(actual_bin_index == expected_bin_index, "max mag peak after HPS for fundamental freq %.2f found at bin index "
						       "%d but expected at index %d", fundamental_freq, actual_bin_index, expected_bin_index);
	Assert(actual_mag == expected_mag, "max mag peak after HPS for fundamental freq %.2f expected mag %.2f but was %.2f", fundamental_freq,
					   expected_mag, actual_mag);
}

static void test_hps_find_harmonic_peaks(void)
{
	assert_hps_find_harmonic_peaks(82.01, 1);
	assert_hps_find_harmonic_peaks(82.41, 1);
	assert_hps_find_harmonic_peaks(83, 1);
}

/* TODO rename inline to whatever else gets added to note.h */
static float32_t note_frequency(const char *note_name)
{
	struct note_freq *nf = note_freqs;
	
	for (; nf->note_name; ++nf) {
		if (strcasecmp(note_name, nf->note_name) == 0) {
			return nf->frequency;
		}
	}
	return 0;
}

/* Assert harmonic product spectrum turns the fundamental frequency into the maximum peak. */
static bool assert_hps(const char *note_name, int i, const int16_t *samples, enum frame_length frame_len)
{
	float32_t *freq_bin_magnitudes;
	float32_t note_freq;
	int expected_bin_index, actual_bin_index;

	freq_bin_magnitudes = samples_to_freq_bin_magnitudes(samples, frame_len);
	harmonic_product_spectrum(freq_bin_magnitudes, frame_len);

	note_freq = note_frequency(note_name);
	expected_bin_index = freq_to_bin_index(note_freq, bin_width(frame_len));
	actual_bin_index = max_bin_index(freq_bin_magnitudes, frame_len);

	Assert(expected_bin_index == actual_bin_index, "expected bin index %d for note %s (%.3f Hz), frame len %d, frame %d, but was %d",
							expected_bin_index, note_name, note_freq, frame_len, i, actual_bin_index);
	return true;
}

static bool semitone_cents_diff_within_tolerance(float32_t cents_diff)
{
	const float32_t tolerance = 0.1;
	/* TODO abs won't work with q notation? */
	return fabs(fabs(cents_diff) - CENTS_IN_SEMITONE) <= tolerance;
}

/* 
 * Assert each pair of adjacent notes in note_freqs is about CENTS_IN_SEMITONE 
 * (within some tolerance) apart from each other. 
 */
static void test_cents_difference(void)
{
	struct note_freq *prev, *cur, *next;

	prev = NULL;
	cur = note_freqs;
	while (cur->note_name) {
		if (prev) {
			float32_t cents_diff = cents_difference(prev->frequency, cur->frequency);
			Assert(cents_diff > 0, "prev %s, cur %s, cents diff %.3f", prev->note_name, cur->note_name, cents_diff);
			Assert(semitone_cents_diff_within_tolerance(cents_diff), "prev %s, cur %s, cents diff %.3f", 
										 prev->note_name, cur->note_name, cents_diff);
		}
		next = cur+1;
		if (next->note_name) {
			float32_t cents_diff = cents_difference(next->frequency, cur->frequency);
			Assert(cents_diff < 0, "next %s, cur %s, cents diff %.3f", next->note_name, cur->note_name, cents_diff);
			Assert(semitone_cents_diff_within_tolerance(cents_diff), "next %s, cur %s, cents diff %.3f", 
										 next->note_name, cur->note_name, cents_diff);
		}
		prev = cur++;
	}
}

/* TODO explain (running assert tests) */
int main(void)
{
	/* TODO fix up whitespace */
	for_each_sine_file_source(FRAME_LEN_2048, assert_sine_wave_freq_to_bin_index);
	for_each_sine_file_source(FRAME_LEN_4096, assert_sine_wave_freq_to_bin_index);

	test_hps_find_harmonic_peaks();

	/* TODO test other frame lens :( */
	for_each_note_file_source(FRAME_LEN_4096, assert_hps);

	test_cents_difference();

	return !print_asserts_summary();
}