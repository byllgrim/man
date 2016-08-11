/* see LICENSE file for copyright and license details */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

/* macros */
#define LENGTH(x) (sizeof x / sizeof x[0])

/* function declarations */
static void die(const char *fmt, ...);
static void *ecalloc(size_t nmemb, size_t size);

static void openpage(void);

/* variables */
static char *page = NULL;
static char *section[] = {"1", "2", "3", "4", "5", "6", "7", "8", NULL};

/* function definitions */
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

void *
ecalloc(size_t nmemb, size_t size)
{
	void *p;

	if (!(p = calloc(nmemb, size)))
		perror(NULL);
	return p;
}

void
openpage(void) /* TODO $MANPATH env */
{
	size_t i, j;
	char *path;

	path = ecalloc(BUFSIZ + 1, sizeof(char));
	for (i = 0; i < LENGTH(manpath); i++) {
		for (j = 0; section[j]; j++) {
			/* TODO make prettier */
			strncpy(path, manpath[i], BUFSIZ/2);
			strcat(path, "man");
			strncat(path, section[j], BUFSIZ/4);
			strcat(path, "/");
			strncat(path, page, BUFSIZ/4);
			strcat(path, ".");
			strncat(path, section[j], BUFSIZ/4);
			printf("%s\n", path);
		}
	}

	/* TODO open file for reading */
	free(path);
}

int
main(int argc, char *argv[])
{
	if (argc < 2 || argc > 3)
		die("usage: man [section] page\n");
	if (argc == 3) {
		page = argv[2];
		section[0] = argv[1];
		section[1] = NULL;
	} else {
		page = argv[1];
	}

	openpage(); /* TODO fail dies with strerror */
	/* TODO loadpage()? */
	/* TODO openpipe()? initoutput()? */
	/* TODO printpage()? */

	return EXIT_SUCCESS;
}
