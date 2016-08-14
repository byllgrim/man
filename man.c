/* see LICENSE file for copyright and license details */
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
/* TODO ensure no unneeded includes */

#include "config.h"

/* macros */
#define LENGTH(x) (sizeof x / sizeof x[0])

/* function declarations */
static void die(const char *fmt, ...);
static void *ecalloc(size_t nmemb, size_t size);

static void openfile(void);
static char *makepath(char *path, char *manpath, char *section);
static void openpipe(void);
static void showpage(void);

/* variables */
static char *page = NULL;
static char *section[] = {"1", "2", "3", "4", "5", "6", "7", "8", NULL};
static FILE *output = NULL;
static FILE *manfile = NULL;

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
openfile(void) /* TODO $MANPATH env */
{
	size_t i, j;
	char *path;

	path = ecalloc(BUFSIZ + 1, sizeof(char));
	for (i = 0; i < LENGTH(manpaths); i++) {
		for (j = 0; section[j]; j++) {
			path = makepath(path, manpaths[i], section[j]);
			if ((manfile = fopen(path, "r")))
				goto done;
			/* TODO gziped man pages? */
		}
	}
done:
	free(path);
	if (!manfile)
		die("mvi: error: %s\n", strerror(errno));
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

void
openpipe(void)
{
	size_t i;

	errno = 0;
	for (i = 0; !output && i < LENGTH(pagers); i++)
		output = popen(pagers[i], "w");

	if (!output)
		die("man: pager: error: %s\n", strerror(errno));
}

void
showpage(void)
{
	char s[BUFSIZ];

	fgets(s, BUFSIZ, manfile);
	while (!feof(manfile)) {
		fprintf(output, "%s", s);
		fgets(s, BUFSIZ, manfile);
		/* TODO parse content */
	}
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

	openfile();
	openpipe();
	showpage();

	pclose(output);
	return EXIT_SUCCESS;
}
