/*
 * Copyright (C) 2024 Petar Turukalo
 * SPDX-License-Identifier: GPL-2.0
 */
#ifndef FILE_SOURCE_H
#define FILE_SOURCE_H

#include <stdbool.h>
#include "dsp.h"

#define NOTE_FILES_DIR "data/note"
#define SINE_FILES_DIR "data/sine"

/* 
 * Function that processes a frame of samples from a file source. 
 * Return false on error.
 */
typedef bool (*process_samples_fn)(
	const char *filename,  /* Excluding .raw extension. */
	int i,  /* 1-indexed frame number from the start of the file source. */
	const int16_t *samples, 
	enum frame_length frame_len);  /* Number of samples. */

/*
 * Run file_source() on each file source in the NOTE_FILES_DIR or SINE_FILES_DIR
 * directory. See file_source() for more info.
 */
bool for_each_note_file_source(enum frame_length frame_len, process_samples_fn process_samples);
bool for_each_sine_file_source(enum frame_length frame_len, process_samples_fn process_samples);

#endif
