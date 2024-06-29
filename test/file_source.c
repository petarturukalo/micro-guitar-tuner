/*
 * Copyright (C) 2024 Petar Turukalo
 * SPDX-License-Identifier: GPL-2.0
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <libgen.h>
#include "file_source.h"

static bool str_has_suffix(const char *str, const char *suffix)
{
	int n = strlen(str);
	int m = strlen(suffix);

	if (n < m)
		return false;
	for (int i = 0; i < m; ++i) {
		if (str[n-1-i] != suffix[m-1-i])
			return false;
	}
	return true;
}

/* Get the size of a file in bytes, or -1 on error. */
static int file_size(int fd)
{
	struct stat stat;

	if (fstat(fd, &stat) == -1)
		return -1;
	return stat.st_size;
}

/*
 * Run a function on a stream of samples sourced from a file. 
 * See data/{note,sine}/README.md for an explanation of the format of a file source.
 * The samples in the file are split into oversized frames of length 
 * frame_len*OVERSAMPLING_FACTOR samples, and process_samples called once on each 
 * frame until there aren't enough samples left in the file to fill a whole frame.
 */
static bool file_source(const char *pathname, enum frame_length frame_len,
			process_samples_fn process_samples)
{
	int fd, fsz, sample_size = sizeof(int16_t);
	int16_t *samples;
	char *filename;
	int remaining_samples;
	bool ret = true;
	const int oversize_frame_len = frame_len*OVERSAMPLING_FACTOR;

	fd = open(pathname, O_RDONLY);
	if (fd == -1) {
		fprintf(stderr, "Error opening file %s for reading: %s\n", pathname, strerror(errno));
		return false;
	}
	fsz = file_size(fd);
	if (fsz == -1) {
		fprintf(stderr, "Error getting size of file %s: %s\n", pathname, strerror(errno));
		close(fd);
		return false;
	}
	samples = malloc(oversize_frame_len*sample_size);
	if (!samples) {
		fprintf(stderr, "Error allocating memory to store samples in: %s\n", strerror(errno));
		close(fd);
		return false;
	}
	/* Extract name from pathname, excluding .raw extension, e.g. "E2" from "data/E2.raw" */
	filename = basename((char *)pathname);
	*(strrchr(filename, '.')) = '\0';  /* Chop off .raw extension. */

	remaining_samples = fsz/sample_size;

	for (int i = 1; remaining_samples >= oversize_frame_len; ++i) {
		/* Read frame of samples. */
		int bytes_read = read(fd, samples, oversize_frame_len*sample_size);
		if (bytes_read == -1) {
			fprintf(stderr, "Error reading from file source %s: %s\n", filename, strerror(errno));
			ret = false;
			break;
		}
		if (bytes_read != oversize_frame_len*sample_size) {
			fprintf(stderr, "Error: failed to read full frame of samples\n");
			ret = false;
			break;
		}

		if (!process_samples(filename, i, samples, frame_len)) {
			ret = false;
			break;
		}
		remaining_samples -= oversize_frame_len;
	}
	free(samples);
	close(fd);
	return ret;
}

bool for_each_file_source(const char *file_source_dir, enum frame_length frame_len, 
			  process_samples_fn process_samples)
{
	DIR *dir;
	struct dirent *dirent;
	bool ret = true;
	char pathname[64];
	
	dir = opendir(file_source_dir);
	if (!dir) {
		fprintf(stderr, "Error opening directory %s: %s\n", file_source_dir, strerror(errno));
		return false;
	}
	errno = 0;
	while (dirent = readdir(dir)) {
		if (!str_has_suffix(dirent->d_name, ".raw"))
			continue;
		snprintf(pathname, sizeof(pathname), "%s/%s", file_source_dir, dirent->d_name);
		if (!file_source(pathname, frame_len, process_samples))
			break;
		errno = 0;
	}
	if (errno) {
		fprintf(stderr, "Error reading from directory %s: %s\n", file_source_dir, strerror(errno));
		ret = false;
	}
	closedir(dir);
	return ret;
}

