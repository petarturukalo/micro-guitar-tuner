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

#endif
