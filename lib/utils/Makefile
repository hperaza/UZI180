# Makefile to build Linux utilities to check UZI180 libraries.

SRCS = checklib.c sortlib.c

all: checklib sortlib

checklib: checklib.c
	cc -o $@ $<

sortlib: sortlib.c
	cc -o $@ $<

clean:
	rm -f checklib sortlib *~ core *.\$$\$$\$$
