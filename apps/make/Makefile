# Makefile for make (!)

.SUFFIXES: .c .obj

CFLAGS = -v -Duzi

SRCS = main.c \
	check.c \
	input.c \
	macro.c \
	make.c \
	make1.c \
	docmd.c \
	reader.c \
	rules.c \
	archive.c \
	archive1.c

OBJS = $(SRCS:.c=.obj)

LIBS = uzilibc.lib

.c.obj:
	zxc -o -c $(CFLAGS) $<

all: make

make: $(OBJS) h.h
	@echo "-Z -W3 -Ptext=0,data,bss -C100h -o"$@" crt.obj \\" > linkmake
	@echo $(OBJS)" \\" >> linkmake
	@echo "uzilibc.lib" >> linkmake
	zxcc link -"<" +linkmake

clean:
	rm -f make linkmake $(OBJS) *.bak core *~ *.\$$\$$\$$

rmbak:
	rm -f *.bak core *~ *.\$$\$$\$$
