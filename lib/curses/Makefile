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

SRCS	= beep.c charpick.c cursset.c cursesio.c endwin.c flash.c \
	  initscr.c longname.c move.c mvcursor.c newwin.c options.c \
	  overlay.c prntscan.c refresh.c scrreg.c setterm.c tabsize.c \
	  termmisc.c unctrl.c update.c waddch.c waddstr.c wbox.c \
	  wclear.c wclr2bot.c wclr2eol.c wdelch.c wdelln.c werase.c \
	  wgetch.c wgetstr.c windel.c winmove.c winsch.c winscrol.c \
	  winsln.c wintouch.c termcap.c

OBJS	= $(SRCS:.c=.obj)

LIB	= ulibcurs.lib

all:	$(LIB)

$(LIB):	$(OBJS)
	rm -f $(LIB)
	$(LIBR) -"<" +mklib
	$(LIBR) -s $(LIB) > ulibcurs.map

clean:
	rm -f $(LIB) *.obj *.map *~ core *.\$$\$$\$$
