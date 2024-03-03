#ifndef ASSERT_H
#define ASSERT_H

#include <stdbool.h>

/*
 * Assert whether the condition is true. If false print the error message
 * to stderr, and increment the count of failed assertions. Pass NULL to
 * format to not print an error message on error.
 */
void Assert(bool condition, const char *function, const char *file, int line,
	    const char *format, ...);
#define Assert(condition, format, ...)  Assert(condition, __func__, __FILE__, __LINE__, \
					       format __VA_OPT__(,) __VA_ARGS__)

/*
 * Print whether all asserts run so far have been successful, along 
 * with the count of failed asserts and total count of asserts.
 * Return whether all asserts were successful.
 */
bool print_asserts_summary(void);

#endif
