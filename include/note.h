#ifndef NOTE_H
#define NOTE_H

#include <arm_math_types.h>

struct note_freq {
	const char *note_name;
	float32_t frequency;
};

/* Table of notes and their frequencies to 3 decimal places. */
extern struct note_freq note_freqs[];

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
 * TODO explain log scale, etc?
 * TODO reference this function was taken from the digital audio with java book
 * TODO should be returning int because cents is int, but that would ruin the tests?
 */
float32_t cents_difference(float32_t frequency, struct note_freq *reference);

/* 
 * Get the note closest to the input frequency. The returned note will be at most 
 * CENTS_IN_HALF_SEMITONE cents away from the frequency. Return NULL if the frequency 
 * is not in the covered frequency range.
 */
struct note_freq *nearest_note(float32_t frequency);

#endif
