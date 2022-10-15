# Makefile for curses.

.SUFFIXES:	.c .obj .as

LIBR	= zxcc libr
CC	= zxc
CFLAGS	= -o -DUZI
AS	= zxas
ASFLAGS	= -j -n

.c.obj:
	$(CC) $(CFLAGS) -c $<

.as.obj:
	$(AS) $(ASFLAGS) $<

SRCS	= tgetent.c tgetflag.c tgetnum.c tgetstr.c tgoto.c tputs.c

OBJS	= $(SRCS:.c=.obj)

LIB	= ulibtcap.lib

all:	$(LIB)

$(LIB):	$(OBJS)
	rm -f $@
	$(LIBR) -"<" +mklib
	$(LIBR) -s $@ > ulibtcap.map

clean:
	rm -f $(LIB) *.obj *.map *~ core *.\$$\$$\$$
