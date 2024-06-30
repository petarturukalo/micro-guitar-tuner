/*
 * Copyright (C) 2024 Petar Turukalo
 * SPDX-License-Identifier: GPL-2.0
 */
#include "dsp_indirect.h"
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
static bool assert_sine_wave_freq_to_bin_index(const char *sine_freq_str, int i, const int16_t *samples, enum frame_length frame_len)
{
	float32_t sine_freq;
	float32_t *freq_bin_magnitudes;
	int expected_bin_index, actual_bin_index;

	sscanf(sine_freq_str, "%f", &sine_freq);
	if (i == 1)
		samples_to_freq_bin_magnitudes_init(frame_len);
	freq_bin_magnitudes = samples_to_freq_bin_magnitudes_s16(samples, frame_len);
	expected_bin_index = freq_to_bin_index(sine_freq, bin_width(frame_len, SAMPLING_RATE));
	actual_bin_index = max_bin_index(freq_bin_magnitudes, frame_len);

	Assert(expected_bin_index == actual_bin_index, "expected sine %.2f Hz at bin index %d but was %d, bin width %.3f", 
							sine_freq, expected_bin_index, actual_bin_index, bin_width(frame_len, SAMPLING_RATE));
	return true;
}

/*
 * Generate peaks in the empty magnitudes array at the first NHARMONICS harmonics of the fundamental
 * frequency. Return the expected magnitude value of the fundamental frequency / maximum peak after HPS.
 */
static float32_t generate_harmonic_peaks(float32_t fundamental_freq, float32_t *mags, float32_t binwidth)
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
static void assert_hps_find_harmonic_peaks(float32_t fundamental_freq, enum frame_length frame_len, int sampling_rate)
{
	float32_t mags[MAX_NR_BINS] = { 0 };
	int expected_bin_index, actual_bin_index;
	float32_t expected_mag, actual_mag;
	float32_t binwidth;

	binwidth = bin_width(frame_len, sampling_rate);
	expected_bin_index = freq_to_bin_index(fundamental_freq, binwidth);
	expected_mag = generate_harmonic_peaks(fundamental_freq, mags, binwidth);
	harmonic_product_spectrum(mags, frame_len, sampling_rate);
	actual_bin_index = max_bin_index(mags, frame_len);
	actual_mag = mags[actual_bin_index];
	
	Assert(actual_bin_index == expected_bin_index, "max mag peak after HPS for fundamental freq %.2f found at bin index "
						       "%d but expected at index %d", fundamental_freq, actual_bin_index, expected_bin_index);
	Assert(actual_mag == expected_mag, "max mag peak after HPS for fundamental freq %.2f expected mag %.2f but was %.2f", fundamental_freq,
					   expected_mag, actual_mag);
}

static void test_hps_find_harmonic_peaks(void)
{
	/* Reminder bin width will be ~0.9766. */
	assert_hps_find_harmonic_peaks(97.7, FRAME_LEN_4096, SAMPLING_RATE);
	assert_hps_find_harmonic_peaks(98, FRAME_LEN_4096, SAMPLING_RATE);
	assert_hps_find_harmonic_peaks(98.6, FRAME_LEN_4096, SAMPLING_RATE);
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
	harmonic_product_spectrum(freq_bin_magnitudes, frame_len, SAMPLING_RATE);

	note_freq = note_frequency(note_name);
	expected_bin_index = freq_to_bin_index(note_freq, bin_width(frame_len, SAMPLING_RATE));
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
	char expected_str[32], actual_str[32];

	/* Convert to 2 decimal places to accept some tolerance. */
	snprintf(expected_str, sizeof(expected_str), "%.2f", expected_s16_sample);
	snprintf(actual_str, sizeof(actual_str), "%.2f", actual_s16_sample);
	Assert(strcmp(expected_str, actual_str) == 0, "convert sample %u expected %s but was %s", 
						      u12_sample, expected_str, actual_str);
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


static struct anti_alias_sine {
	float32_t frequency;
	float32_t max_magnitude;
} anti_alias_sines[] = {
/* 
 * Each entry is for a sine wave file in directory `data/sine/anti-alias`,
 * and will have its `max_magnitude` field filled by get_anti_alias_sine_mag().
 */
	{ 200, 0 },
	{ 600, 0 },
	{ 1000, 0 },
	{ 1400, 0 },
	{ 1800, 0 },
/* 
 * Below are below the cutoff frequency and after the end of the cutoff slope 
 * and would alias if not filtered out.
 */
	{ 2200, 0 },
	{ 2600, 0 },
	{ 3000, 0 },
	{ 3400, 0 },
	{ 3800, 0 },
	{ 0, 0 }
};

static struct anti_alias_sine *anti_alias_sine_lookup_by_freq(float32_t frequency)
{
	struct anti_alias_sine *sine = anti_alias_sines;
	for (; sine->frequency; ++sine) {
		if (frequency == sine->frequency)
			return sine;
	}
	return NULL;
}

/* Populate the `max_magnitude` field of the sine in `anti_alias_sines` with matching frequency. */
static bool get_anti_alias_sine_mag(const char *sine_freq_str, int i, const int16_t *samples, enum frame_length frame_len)
{
	float32_t sine_freq;
	float32_t *freq_bin_magnitudes;
	int max_bin_ind;
	struct anti_alias_sine *sine;

	sscanf(sine_freq_str, "%f", &sine_freq);
	sine = anti_alias_sine_lookup_by_freq(sine_freq);
	Assert(sine, "sine wave file %s no entry in anti_alias_sines", sine_freq_str);
	if (sine) {
		if (i == 1)
			samples_to_freq_bin_magnitudes_init(frame_len);
		freq_bin_magnitudes = samples_to_freq_bin_magnitudes_s16(samples, frame_len);
		max_bin_ind = max_bin_index(freq_bin_magnitudes, frame_len);
		sine->max_magnitude = freq_bin_magnitudes[max_bin_ind];
	}
	return true;
}

/* Assert sine waves with frequency above the low-pass filter cutoff frequency do not alias. */
static void test_sine_wave_anti_alias(void)
{
	struct anti_alias_sine *sine;
	float64_t average_non_aliased_mag = 0;
	float32_t aliased_mag_thresh;
	const int lowpass_cutoff_freq = nyquist_frequency(SAMPLING_RATE);
	
	for_each_file_source(SINE_FILES_DIR "/anti-alias", FRAME_LEN_4096, get_anti_alias_sine_mag);
	
	/* Get average max magnitude of all non-aliasing sines. */
	sine = anti_alias_sines;
	for (; sine->frequency <= lowpass_cutoff_freq; ++sine) 
		average_non_aliased_mag += sine->max_magnitude;
	average_non_aliased_mag /= sine-anti_alias_sines;

	/* 
	 * Calculation gotten by looking at graphs of the aliased sines. 
	 * The aliases aren't removed entirely, just small enough to be negligible.
	 */
	aliased_mag_thresh = average_non_aliased_mag/1000;  

	/* Assert all sines with frequency above the cutoff frequency do not alias. */
	for (; sine->frequency; ++sine) {
		Assert(sine->frequency > lowpass_cutoff_freq, "sine %f not above cutoff freq %d", sine->frequency, lowpass_cutoff_freq);
		Assert(sine->max_magnitude <= aliased_mag_thresh, "sine %f did not get aliased", sine->frequency);
	}
}


int main(void)
{
	for_each_file_source(SINE_FILES_DIR "/freq-to-bin-index", FRAME_LEN_4096, assert_sine_wave_freq_to_bin_index);
	test_hps_find_harmonic_peaks();
	for_each_file_source(NOTE_FILES_DIR, FRAME_LEN_4096, assert_hps);
	test_cents_difference();
	test_convert_adc_u12_sample_to_s16();
	test_bit_array_2d_copy();
	test_sine_wave_anti_alias();

	return !print_asserts_summary();
}
