/* TODO if only needed by tests take out of lib? */
#ifndef NOTE_H
#define NOTE_H

#include <arm_math_types.h>

/* 
 * Get the frequency of a note to 3 decimal places.
 * Note name shall be uppercase as lookup is case sensitive, e.g. "E2".
 * Return 0 on error.
 */
float32_t note_frequency(const char *note_name);

#endif
