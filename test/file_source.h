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

/**
 * Function that processes an oversized frame of samples from a file source. 
 * Return false on error.
 */
typedef bool (*process_samples_fn)(
	const char *filename,  /**< Excluding .raw extension. */
	int i,  /**< 1-indexed frame number from the start of the file source. */
	const int16_t *samples, 
	enum frame_length frame_len);  /**< Number of samples is frame_len*OVERSAMPLING_FACTOR. */

/**
 * @brief Run file_source() on each file source in the `file_source_dir` directory. 
 * @see file_source() for more info.
 */
bool for_each_file_source(const char *file_source_dir, enum frame_length frame_len, 
			  process_samples_fn process_samples);

#endif
