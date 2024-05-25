/*
 * Copyright (C) 2024 Petar Turukalo
 * SPDX-License-Identifier: GPL-2.0
 */
#ifndef NOTE_H
#define NOTE_H

#include <arm_math_types.h>

struct note_freq {
	const char *note_name;
	float32_t frequency;
};

/* Table of notes and their frequencies to 3 decimal places. */
extern struct note_freq note_freqs[];
extern struct note_freq null_nf;

/*
 * Get the frequency of the lowest note C0.
 */
float32_t lowest_note_frequency(void);

#define CENTS_IN_OCTAVE  1200
#define CENTS_IN_SEMITONE 100
#define CENTS_IN_HALF_SEMITONE (CENTS_IN_SEMITONE/2)

/*
 * Get in cents how close a frequency is to a reference note frequency.
 * A positive return means the frequency is higher than the reference, and 
 * a negative return means it's lower.
 *
 * A cent is one one hundredth of the difference between two adjacent notes a
 * semitone apart; the difference between two adjacent notes is CENTS_IN_SEMITONE cents. 
 *
 * Why difference in cents is used (over, say, difference in frequency): the pitch 
 * of a note is our perception of its frequency. The pitch of the same note an octave 
 * higher is at twice the frequency of the initial note; thus this perception is base-2 
 * logarithmic. This means the difference in frequency between any two notes that we 
 * perceive to be the same distance apart as another two notes, isn't necessarily the 
 * same distance in frequency. So cents is used because the difference in cents reflects
 * our perception.
 *
 * TODO reference this function was taken from the digital audio with java book
 */
int cents_difference(float32_t frequency, struct note_freq *reference);

/* 
 * Get the note closest to the input frequency. The returned note will be at most 
 * CENTS_IN_HALF_SEMITONE cents away from the frequency. Return NULL if the frequency 
 * is not in the covered frequency range.
 */
struct note_freq *nearest_note(float32_t frequency);

#endif
