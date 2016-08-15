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

static void parseprint(char *s);
static void normalprint(char *s);
static void titleprint(char *s);
static void indent(void);

/* variables */
static char *page = NULL;
static char *section[] = {"1", "2", "3", "4", "5", "6", "7", "8", NULL};
static FILE *output = NULL;
static FILE *manfile = NULL;
static size_t hpos = 0;

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
	section[0] = section[j];
	section[1] = NULL;
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
		if (s[0] == '.')
			parseprint(s);
		else
			normalprint(s);

		fgets(s, BUFSIZ, manfile);
	}
}

void
parseprint(char *s)
{
	char *cmd;
	char *saveptr;

	cmd = strtok_r(s, " \n", &saveptr);

	if (!strcmp(cmd, ".TH")) { /* TODO is strncmp needed? */
		titleprint(strtok_r(NULL, " \n", &saveptr));
	} else if (!strcmp(cmd, ".SH")) {;
		fprintf(output, "\n\n%s", saveptr);
		hpos = 0;
	} else if (!strcmp(cmd, ".P")) { /* TODO or .LP or .PP */
		fprintf(output, "\n\n");
		hpos = 0;
	} else {
		if (saveptr[0] != '\0')
			normalprint(saveptr);
	}
}

void
normalprint(char *s)
{
	int i;

	for (i = 0; hpos <= MANWIDTH && s[i] != '\0'; ++i) {
		if (!hpos)
			indent();
		if (s[i] == '\\') /* TODO while? */
			i++;
		if (s[i] == '\n')
			s[i] = ' ';

		fputc(s[i], output);
		hpos++;

		if (hpos >= MANWIDTH) /* shouldnt be bigger. ever. */
			hpos = 0;
	}
}

void
titleprint(char *s)
{
	size_t padding, i;

	fprintf(output, "%s(%s)", s, section[0]);

	padding = MANWIDTH - 2*strlen(s) - 6; /* TODO section name? */
	for (i = 0; i < padding; i++)
		fputc(' ', output);

	fprintf(output, "%s(%s)", s, section[0]);
}

void
indent(void)
{
	/* less than hpos%MANWIDTH + something */
	int i;

	for (i = 0; i < TABSTOP; i++)
		fputc(' ', output);

	hpos += TABSTOP;
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
