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

$(LIB):	$(OBJS)
	-rm -f $(LIB)
	$(LIBR) -"<" +mklib
	$(LIBR) -s $(LIB) > uuni.map

clean:
	rm -f $(LIB) *.obj *.map *~ core *.\$$\$$\$$
