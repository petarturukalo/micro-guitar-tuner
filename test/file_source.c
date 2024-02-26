/* TODO gnuradio depends on libsndfile for this sort of thing? */
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
#include "file_source.h"

/*
 * TODO if need general audio file source functionality (not just from note) 
 * in future then refactor:
 * - rename note_file_source to something like audio_file_source
 * - don't pass note name to process_samples_fn
 * - rename note-recordings dir to general like audio-recordings and then maybe
 *   a sub dir for notes audio-recordings/notes. split up its READMEs appropriately,
 *   e.g. dir has README for all 
 * ...
 */

/* TODO don't reinvent the wheel for tests. use c++? */
static bool str_has_suffix(const char *str, const char *suffix)
{
	int i, j;
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
 * Run a function on a stream of samples sourced from a note file. 
 * See note-recordings/README.md for an explanation of the format of a note file.
 * The samples in the file are split into frames of size frame_len samples, 
 * and process_samples called once on each frame until there aren't enough 
 * samples left in the file to fill a whole frame.
 */
static bool note_file_source(const char *pathname, enum frame_length frame_len,
			     process_samples_fn process_samples)
{
	int fd, fsz, sample_size = sizeof(int16_t);
	int16_t *samples;
	char note_name[16];
	int remaining_samples;
	bool ret = true;

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
	samples = malloc(frame_len*sample_size);
	if (!samples) {
		fprintf(stderr, "Error allocating memory to store samples in: %s\n", strerror(errno));
		close(fd);
		return false;
	}
	/* Extract note name from pathname, e.g. "E2" from "data/E2.raw" */
	sscanf(pathname, "%*[^/]/%[^.]", note_name);
	remaining_samples = fsz/sample_size;

	for (int i = 1; remaining_samples >= frame_len; ++i) {
		/* Read frame of samples. */
		int bytes_read = read(fd, samples, frame_len*sample_size);
		if (bytes_read == -1) {
			fprintf(stderr, "Error reading from file %s: %s\n", pathname, strerror(errno));
			ret = false;
			break;
		}
		if (bytes_read != frame_len*sample_size) {
			fprintf(stderr, "Error: failed to read full frame of samples\n");
			ret = false;
			break;
		}

		if (!process_samples(note_name, i, samples, frame_len)) {
			ret = false;
			break;
		}
		remaining_samples -= frame_len;
	}
	free(samples);
	close(fd);
	return ret;
}

bool for_each_note_file_source(enum frame_length frame_len, process_samples_fn process_samples)
{
	DIR *dir;
	struct dirent *dirent;
	bool ret = true;
	char pathname[64];
	
	dir = opendir(NOTE_FILES_DIR);
	if (!dir) {
		fprintf(stderr, "Error opening directory %s: %s\n", NOTE_FILES_DIR, strerror(errno));
		return false;
	}
	errno = 0;
	while (dirent = readdir(dir)) {
		if (!str_has_suffix(dirent->d_name, ".raw"))
			continue;
		snprintf(pathname, sizeof(pathname), "%s/%s", NOTE_FILES_DIR, dirent->d_name);
		if (!note_file_source(pathname, frame_len, process_samples))
			break;
		errno = 0;
	}
	if (errno) {
		fprintf(stderr, "Error reading from directory %s: %s\n", NOTE_FILES_DIR, strerror(errno));
		ret = false;
	}
	closedir(dir);
	return ret;
}

