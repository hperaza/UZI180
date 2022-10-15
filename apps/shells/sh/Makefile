# Makefile for sh

.SUFFIXES: .c .obj

CFLAGS =

SRCS = sh.c \
	var.c \
	gmatch.c \
	area.c \
	csyn.c \
	exec.c \
	run.c \
	do.c \
	eval.c \
	glob.c \
	here.c \
	io.c \
	word.c \
	yylex.c \
	utils.c

OBJS = $(SRCS:.c=.obj)

LIBS = uzilibc.lib

.c.obj:
	zxc -o -c -v $(CFLAGS) $<

all:	sh

sh:	$(OBJS)
	zxlink <linksh

clean:
	rm -f sh $(OBJS) *.bak core *~ *.\$$\$$\$$

rmbak:
	rm -f *.bak core *~ *.\$$\$$\$$
