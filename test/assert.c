/*
 * Copyright (C) 2024 Petar Turukalo
 * SPDX-License-Identifier: GPL-2.0
 */
#include <stdio.h>
#include <stdarg.h>
#include "assert.h"

#undef Assert

static int failed_asserts_count = 0;
static int total_asserts_count = 0;

void Assert(bool condition, const char *function, const char *file, int line,
	    const char *format, ...)
{
	if (!condition) {
		fprintf(stderr, "Assert failed in %s at '%s:%d'%s", function, file, line,
				format ? ": " : "");
		if (format) {
			va_list ap;
			va_start(ap, format);
			vfprintf(stderr, format, ap);
			va_end(ap);
		}
		fprintf(stderr, "\n");

		++failed_asserts_count;
	}
	++total_asserts_count;
}

bool print_asserts_summary(void)
{
	if (failed_asserts_count == 0) {
		printf("Asserts successful: all %d asserts ok\n", total_asserts_count);
		return true;
	} else {
		fprintf(stderr, "Asserts unsuccessful: %d out of %d asserts failed\n", 
			failed_asserts_count, total_asserts_count);
		return false;
	}
}
