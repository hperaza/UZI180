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

OBJS	= assert.obj printf.obj fprintf.obj sprintf.obj doprnt.obj \
	  gets.obj puts.obj fwrite.obj getw.obj putw.obj getenv.obj \
	  putchar.obj perror.obj sys_err.obj fputc.obj flsbuf.obj \
	  fopen.obj freopen.obj fseek.obj fread.obj rewind.obj remove.obj \
	  fdopen.obj setbuf.obj fscanf.obj ctime.obj cgets.obj cputs.obj \
	  sscanf.obj scanf.obj doscan.obj ungetc.obj putenv.obj \
	  fgetc.obj filbuf.obj stdclean.obj fclose.obj fflush.obj \
	  va_arg.obj vprintf.obj vfprintf.obj vsprintf.obj vscanf.obj \
	  vsscanf.obj tmpnam.obj buf.obj exit.obj

SRCS	= fgetc.as fputc.as assert.c buf.c cgets.c stdclean.c \
	  cputs.c ctime.c doprnt.c doscan.c exit.c putenv.c \
	  fclose.c fflush.c filbuf.c flsbuf.c fopen.c fprintf.c \
	  fread.c freopen.c fdopen.c fscanf.c fseek.c fwrite.c \
	  getenv.c gets.c getw.c perror.c printf.c putchar.c \
	  puts.c putw.c remove.c rewind.c scanf.c setbuf.c tmpnam.c \
	  va_arg.c vprintf.c vfprintf.c vsprintf.c vscanf.c vsscanf.c \
	  setup.c sprintf.c sscanf.c stdio.h stdio.i sys_err.c ungetc.c

LIB	= ustdio.lib

all:	$(LIB)

$(LIB):	$(OBJS)
	-rm -f $@
	$(LIBR) -"<" +mklib
	$(LIBR) -s $@ > ustdio.map

fgetc.obj: fgetc.as

clean:
	rm -f $(LIB) *.obj *.map *~ core *.\$$\$$\$$
