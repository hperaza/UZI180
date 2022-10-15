# Makefile for building find utility

.SUFFIXES: .c .obj

CFLAGS =

SRCS = main.c \
	find.c \
	check.c \
	exec.c \
	parser1.c \
	parser2.c \
	parser3.c \
	utils.c

OBJS = $(SRCS:.c=.obj)

LIBS = uzilibc.lib

.c.obj:
	zxc -o -c -v $(CFLAGS) $<

all:	find

find:	$(OBJS)
	@echo "-Z -W3 -Ptext=0,data,bss -C100h -o"$@" \\" > linkfind
	@echo "crt.obj "$(OBJS)" \\" >> linkfind
	@echo "uzilibc.lib" >> linkfind
	zxlink <linkfind

clean:
	rm -f find linkfind $(OBJS) *.bak core *~ *.\$$\$$\$$

rmbak:
	rm -f *.bak core *~ *.\$$\$$\$$
