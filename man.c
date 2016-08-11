/* see LICENSE file for copyright and license details */
#include <errno.h>
#include <fcntl.h>
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

static int openpage(void);
static char *makepath(char *path, char *manpath, char *section);

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

int
openpage(void) /* TODO $MANPATH env */
{
	size_t i, j;
	char *path;
	int fd = -1;

	path = ecalloc(BUFSIZ + 1, sizeof(char));
	errno = 0;
	for (i = 0; i < LENGTH(manpath); i++) {
		for (j = 0; section[j]; j++) {
			path = makepath(path, manpath[i], section[j]);
			printf("%s\n", path);
			if ((fd = open(path, O_RDONLY)) != -1)
				goto done;
			/* TODO gziped man pages? */
		}
	}
done:
	free(path);
	return fd;
}

char *
makepath(char *path, char *manpath, char *section)
{
	strncpy(path, manpath, BUFSIZ/2);

	strcat(path, "man");
	strncat(path, section, BUFSIZ/4);
	strcat(path, "/");

	strncat(path, page, BUFSIZ/4);
	strcat(path, ".");
	strncat(path, section, BUFSIZ/4);

	return path;
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

	if (openpage() < 0)
		die("mvi: error: %s\n", strerror(errno));
	else
		printf("wohoo\n");
	/* TODO loadpage()? */
	/* TODO openpipe()? initoutput()? */
	/* TODO printpage()? */

	return EXIT_SUCCESS;
}
