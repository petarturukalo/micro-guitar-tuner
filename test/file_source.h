#ifndef FILE_SOURCE_H
#define FILE_SOURCE_H

#include <stdbool.h>
#include "dsp.h"

#define NOTE_FILES_DIR "note-recordings"

/* 
 * Function that processes a frame of samples from a note file source. 
 * Return false on error.
 */
typedef bool (*process_samples_fn)(
	const char *note_name,
	int i,  /* 1-indexed frame number from the start of the note file. */
	const int16_t *samples, 
	enum frame_length frame_len);  /* Number of samples. */

/*
 * Run note_file_source() on each note file in the NOTE_FILES_DIR 
 * directory. See note_file_source() for more info.
 */
bool for_each_note_file_source(enum frame_length frame_len, process_samples_fn process_samples);

#endif
