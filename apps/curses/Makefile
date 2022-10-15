.SUFFIXES: .c .obj

SRCS  = main.c \
	life.c \
	makscr.c

OBJS  = $(SRCS:.c=.obj)

LIBS = ulibcurs.lib uzilibc.lib

all: life

$(OBJS): %.obj: %.c

.c.obj:
	zxc -o -c -v -DUZI $<

life: $(OBJS)
	@echo "-Z -W3 -Ptext=0,data,bss -C100h -o"$@ "\\" > linkcmd
	@echo "crt.obj \\" >> linkcmd
	@echo $(OBJS) "\\" >> linkcmd
	@echo $(LIBS) >> linkcmd
	zxcc link -"<" +linkcmd
	@rm -f linkcmd

clean:
	rm -f $(OBJS) life core *~ *.\$$\$$\$$

rmbak:
	rm -f *~ core *.\$$\$$\$$
