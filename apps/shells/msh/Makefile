# Minimal shell "make" source

CC	= zxc
CFLAGS	= -DUZI -o -v

SRCS = msh.c envvar.c filmatch.c

OBJS = $(SRCS:.c=.obj)

.SUFFIXES: .c .obj

.c.obj: %.obj: %.c
	zxc $(CFLAGS) -c $<

all: msh lmsh

msh: $(OBJS)
	@echo "-Ptext=0,data,bss -C100h -o"$@ "\\" > linkcmd
	@echo "crt.obj \\" >> linkcmd
	@echo $(OBJS) "\\" >> linkcmd
	@echo uzilibc.lib >> linkcmd
	zxcc link -"<" +linkcmd
	@rm -f linkcmd

lmsh: $(SRCS)
	cc -g -o $@ $(SRCS)

clean:
	rm -f *~ $(OBJS) core msh lmsh
