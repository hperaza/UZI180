CC      = gcc
XINC    = -I.
LDFLAGS = -L../lib
AR      = ar
RANLIB  = ranlib

CFLAGS  = -g $(XINC)

SRCS    = devio.c \
	  devflop.c \
	  devhd.c \
	  xdata.c \
	  xdevmisc.c \
	  xdevtty.c \
	  xfs1.c \
	  xfs1a.c \
	  xfs1b.c \
	  xfs2.c \
	  xmachdep.c \
	  flopimg.c

OBJS    = $(SRCS:.c=.o)

LIB     = libuzi.a

all:	$(LIB)

.SUFFIX: .c .o

.c.o:
	$(CC) $(CFLAGS) -c $<

$(LIB):	$(OBJS)
	$(AR) cr $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

clean:
	rm -f $(OBJS) $(LIB) core *~ *_
