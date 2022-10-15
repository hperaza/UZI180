.SUFFIXES:	.c .obj .as

LIBR	= zxcc libr
C	= zxc
CFLAGS	= -o -v
AS	= zxas
ASFLAGS	= -j -n

.c.obj:
	$(C) $(CFLAGS) -c $<
.as.obj:
	$(AS) $(ASFLAGS) $<

SRCS	= convdate.c getcwd.c getpw.c putpw.c getgr.c putgr.c \
	  dirent.c crypt.c getpass.c isatty.c

OBJS	= $(SRCS:.c=.obj)

LIB	= uuni.lib

all:	$(LIB)

$(LIB):	$(OBJS)
	-rm -f $@
	$(LIBR) -"<" +mklib
	$(LIBR) -s $@ > uuni.map

clean:
	rm -f $(LIB) *.obj *.map *~ core *.\$$\$$\$$
