/*
 * Copyright (C) 2024 Petar Turukalo
 * SPDX-License-Identifier: GPL-2.0
 */
#include "note.h"
#include <dsp/fast_math_functions.h>
#include <math.h>

struct note_freq note_freqs[] = {
	{ "C0",  16.351 },
	{ "C#0", 17.324 },
	{ "D0",  18.354 },
	{ "D#0", 19.445 },
	{ "E0",  20.601 },
	{ "F0",  21.827 },
	{ "F#0", 23.124 },
	{ "G0",	 24.499 },
	{ "G#0", 25.956 },
	{ "A0",  27.5 },
	{ "A#0", 29.135 },
	{ "B0",  30.868 },

	{ "C1",  32.703 },
	{ "C#1", 34.648 },
	{ "D1",  36.708 },
	{ "D#1", 38.891 },
	{ "E1",  41.203 },
	{ "F1",  43.654 },
	{ "F#1", 46.249 },
	{ "G1",  48.999 },
	{ "G#1", 51.913 },
/* 
 * Starting at A1 is the lowest note covered by tests because it's the 
 * lowest you'd realistically tune down to (or not?). Notes above this
 * comment (but "lower" in pitch) are only handled (and with poor frequency 
 * resolution) to get a gist of the pitch when tuning up for the first 
 * time after putting on a new set of strings.
 */
	{ "A1",  55 },
	{ "A#1", 58.27 },
	{ "B1",  61.735 },

	{ "C2",  65.406 },
	{ "C#2", 69.296 },
	{ "D2",  73.416 },
	{ "D#2", 77.782 },
	{ "E2",  82.407 },
	{ "F2",  87.307 },
	{ "F#2", 92.499 },
	{ "G2",  97.999 },
	{ "G#2", 103.826 },
	{ "A2",  110 },
	{ "A#2", 116.541 },
	{ "B2",  123.471 },

	{ "C3",  130.813 },
	{ "C#3", 138.591 },
	{ "D3",  146.832 },
	{ "D#3", 155.563 },
	{ "E3",  164.814 },
	{ "F3",  174.614 },
	{ "F#3", 184.997 },
	{ "G3",  195.998 },
	{ "G#3", 207.652 },
	{ "A3",  220 },
	{ "A#3", 233.082 },
	{ "B3",  246.942 },

	{ "C4",  261.626 },
	{ "C#4", 277.183 },
	{ "D4",  293.665 },
	{ "D#4", 311.127 },
	{ "E4",  329.628 },
	{ "F4",  349.228 },
	{ "F#4", 369.994 },
/*
 * Ending at F#4 is the highest note covered by tests, chosen because it's a
 * a few semitones above the high open E string. See also the next below comment.
 */
	{ "G4",  391.995 },
	{ "G#4", 415.305 },
/*
 * Ending at G#4 is the highest note expected to be reliably handled because 
 * its 4th harmonic is below the 1700 Hz cutoff frequency defined at 
 * gen_filter_coeffs.m:lowpass_cutoff_freq, and HPS uses 4 harmonics (see NHARMONICS).
 *
 * This doesn't affect getting the open strings in tune, which is the main use
 * case of this tuner, but may cause issues if trying to test the intonation 
 * of the high E string e.g. when checking its pitch at fret 12.
 */
	{ "A4",  440 },
	{ "A#4", 466.164 },
	{ "B4",  493.883 },

	{ "C5",  523.251 },
	{ "C#5", 554.365 },
	{ "D5",  587.33 },
	{ "D#5", 622.254 },
	{ "E5",  659.255 },
	{ "F5",  698.456 },
	{ "F#5", 739.989 },
	{ "G5",  783.991 },
	{ "G#5", 830.609 },
	{ "A5",  880 },
	{ "A#5", 932.328 },
	{ "B5",  987.767 },
					 
	{ "C6",  1046.502 },
	{ "C#6", 1108.731 },
	{ "D6",  1174.659 },
	{ "D#6", 1244.508 },
	{ "E6",  1318.51 },
	{ "F6",	 1396.913 },
	{ "F#6", 1479.978 },
	{ "G6",  1567.982 },
	{ "G#6", 1661.219 },
/*
 * Ending at G#6 is the highest note with fundamental frequency / first harmonic below
 * the cutoff frequency.
 */
	{ 0 }
};

struct note_freq null_nf = { "?", 0 };


float32_t lowest_note_frequency(void)
{
	return note_freqs[0].frequency;
}

int cents_difference(float32_t frequency, struct note_freq *reference)
{
	return round(-CENTS_IN_OCTAVE*log2f(reference->frequency/frequency));
}

struct note_freq *nearest_note(float32_t frequency)
{
	struct note_freq *nf;

	nf = note_freqs;
	/* Not allowing frequencies lower than the lowest note. */
	if (frequency < nf->frequency)
		return NULL;
	++nf;
	for (; nf->note_name; ++nf) {
		if (frequency < nf->frequency) {
			/* Return the closer note. */
			if (cents_difference(frequency, nf) >= -CENTS_IN_HALF_SEMITONE)
				return nf;
			return nf-1;
		}
	}
	/* Not allowing frequencies higher than the highest note. */
	return NULL;
}

