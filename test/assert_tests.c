#include "dsp.h"
#include "adc.h"
#include "note.h"
#include "2d_bit_array.h"
#include "assert.h"
#include "file_source.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Assert the frequency of a sine wave falls into the expected bin. */
/* TODO note 83 Hz is failing bin width 2 for some reason. not 100% certain on rounding */
static bool assert_sine_wave_freq_to_bin_index(const char *sine_freq_str, int i, const int16_t *samples, enum frame_length frame_len)
{
	float32_t sine_freq;
	float32_t *freq_bin_magnitudes;
	int expected_bin_index, actual_bin_index;

	sscanf(sine_freq_str, "%f", &sine_freq);
	if (i == 1)
		samples_to_freq_bin_magnitudes_init(frame_len);
	freq_bin_magnitudes = samples_to_freq_bin_magnitudes_s16(samples, frame_len);
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

	if (i == 1)
		samples_to_freq_bin_magnitudes_init(frame_len);
	freq_bin_magnitudes = samples_to_freq_bin_magnitudes_s16(samples, frame_len);
	harmonic_product_spectrum(freq_bin_magnitudes, frame_len);

	note_freq = note_frequency(note_name);
	expected_bin_index = freq_to_bin_index(note_freq, bin_width(frame_len));
	actual_bin_index = max_bin_index(freq_bin_magnitudes, frame_len);

	Assert(expected_bin_index == actual_bin_index, "expected bin index %d for note %s (%.3f Hz), frame len %d, frame %d, but was %d",
							expected_bin_index, note_name, note_freq, frame_len, i, actual_bin_index);
	return true;
}

/* 
 * Assert each pair of adjacent notes in note_freqs is CENTS_IN_SEMITONE cents apart from each other. 
 */
static void test_cents_difference(void)
{
	struct note_freq *prev, *cur, *next;

	prev = NULL;
	cur = note_freqs;
	while (cur->note_name) {
		if (prev) {
			int cents_diff = cents_difference(cur->frequency, prev);
			Assert(cents_diff > 0, "prev %s, cur %s, cents diff %.3f", prev->note_name, cur->note_name, cents_diff);
			Assert(cents_diff == CENTS_IN_SEMITONE, "prev %s, cur %s, cents diff %.3f", 
								prev->note_name, cur->note_name, cents_diff);
		}
		next = cur+1;
		if (next->note_name) {
			int cents_diff = cents_difference(cur->frequency, next);
			Assert(cents_diff < 0, "next %s, cur %s, cents diff %.3f", next->note_name, cur->note_name, cents_diff);
			Assert(cents_diff == -CENTS_IN_SEMITONE, "next %s, cur %s, cents diff %.3f", 
								 next->note_name, cur->note_name, cents_diff);
		}
		prev = cur++;
	}
}

static void assert_convert_adc_u12_sample_to_s16(uint16_t u12_sample, float32_t expected_s16_sample)
{
	float32_t actual_s16_sample = convert_adc_u12_sample_to_s16(u12_sample);
	char *expected_str, *actual_str;

	/* Convert to 2 decimal places to accept some tolerance. */
	asprintf(&expected_str, "%.2f", expected_s16_sample);
	asprintf(&actual_str, "%.2f", actual_s16_sample);
	Assert(strcmp(expected_str, actual_str) == 0, "convert sample %u expected %s but was %s", 
						      u12_sample, expected_str, actual_str);
	free(expected_str);
	free(actual_str);
}

static void test_convert_adc_u12_sample_to_s16(void)
{
	assert_convert_adc_u12_sample_to_s16(0, INT16_MIN);
	assert_convert_adc_u12_sample_to_s16(4095, INT16_MAX);
}

void assert_bit_array_2d_copy(uint8_t *dest_bit_array, int dest_ncols, int dest_nrows,
			      uint8_t *src_bit_array, int src_ncols, int src_nrows,
			      struct write_coord coord,
			      uint8_t *expected_bit_array)  /* Expected has the same dimensions as dest. */
{
	bit_array_2d_copy(dest_bit_array, dest_ncols, dest_nrows,
			  src_bit_array, src_ncols, src_nrows,
			  coord);
	Assert(memcmp(dest_bit_array, expected_bit_array, (dest_ncols*dest_nrows)/BITS_IN_BYTE) == 0, NULL);
}

static void test_bit_array_2d_copy(void)
{
	/* 
	 * Testing bit_array_2d_copy() implicitly tests the rest of the functions in
	 * file '2d_bit_array.c'.
	 */
	assert_bit_array_2d_copy((uint8_t[]){ 0b00000000,0b00000000,0b00000000,
					      0b00000000,0b00000000,0b00000000 }, 24, 2,
				 (uint8_t[]){ 0b10101010,0b01010101,0b11111111,
					      0b00001111,0b00000000,0b11110000 }, 24, 2,
				 (struct write_coord){ 0, 0 },
				 (uint8_t[]){ 0b10101010,0b01010101,0b11111111,
					      0b00001111,0b00000000,0b11110000 });

	assert_bit_array_2d_copy((uint8_t[]){ 0b11100000,0b00000000,0b00000000,
					      0b11100000,0b00000000,0b00000000,
					      0b11100000,0b00000000,0b00000000 }, 24, 3,
				 (uint8_t[]){ 0b11000000,
					      0b11000000, }, 8, 2,
				 (struct write_coord){ 1, 15 },
				 (uint8_t[]){ 0b11100000,0b00000000,0b00000000,
					      0b11100000,0b00000001,0b10000000,
					      0b11100000,0b00000001,0b10000000 });
}

/* TODO explain (running assert tests) */
int main(void)
{
	/* TODO fix up whitespace */

	/* TODO not currently linking against RFFT frame len 2048 tables */
	/*for_each_sine_file_source(FRAME_LEN_2048, assert_sine_wave_freq_to_bin_index);*/
	for_each_sine_file_source(FRAME_LEN_4096, assert_sine_wave_freq_to_bin_index);

	test_hps_find_harmonic_peaks();

	/* TODO test other frame lens :( */
	for_each_note_file_source(FRAME_LEN_4096, assert_hps);

	test_cents_difference();

	test_convert_adc_u12_sample_to_s16();
	test_bit_array_2d_copy();

	return !print_asserts_summary();
}
