CPPFLAGS = -D_POSIX_C_SOURCE=200112L
CFLAGS = -std=c89 -pedantic-errors -Wall -Wextra -Os ${CPPFLAGS}

all: config.h
	cc -o man ${CFLAGS} man.c

config.h:
	cp config.def.h config.h
