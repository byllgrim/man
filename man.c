/* See LICENSE file for copyright and license details. */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Function declarations */
static void die(const char *fmt, ...);

/* Function definitions */
void
die(const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	if (fmt[0] && fmt[strlen(fmt)-1] == ':') {
		fputc(' ', stderr);
		perror(NULL);
	}

	exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
	if (argc < 2 || argc > 3)
		die("usage: man [section] page\n");

	/* TODO load man page */

	return EXIT_SUCCESS;
}
