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

SRCS	= abs.as allsh.as alrsh.as asallsh.as asalrsh.as asar.as \
	  asdiv.as asladd.as asland.as asll.as asllrsh.as aslmul.as \
	  aslor.as aslr.as aslsub.as aslxor.as asmod.as asmul.as \
	  atoi.as atol.c bitfield.as blkclr.as blkcpy.c bmove.as \
	  calloc.c csv.as ctype.c ctype_.c brelop.as wrelop.as frelop.as \
	  cseek.c getsp.as idiv.as imul.as index.as inout.as \
	  iregset.as isalpha.as isdigit.as islower.as isspace.as isupper.as \
	  ladd.as land.as ldiv.as linc.as llrsh.as lmul.as \
	  longjmp.as lor.as lrelop.as lsub.as lxor.as malloc.c \
	  max.as memchr.c memcmp.c memcpy.c memmove.c memset.c pnum.c  \
	  qsort.c swap.as rand.c rcsv.as rindex.as shar.as \
	  shll.as shlr.as strcat.as strchr.as strcmp.as strcpy.as \
	  strlen.as strncat.as strncmp.as strncpy.as strrchr.as \
	  tolower.as toupper.as xtoi.as

OBJS	= memchr.obj memcpy.obj memmove.obj memcmp.obj memset.obj \
	  abs.obj asallsh.obj allsh.obj asalrsh.obj asar.obj \
	  asdiv.obj asladd.obj asland.obj asll.obj asllrsh.obj \
	  aslmul.obj aslor.obj aslsub.obj aslxor.obj asmod.obj \
	  atoi.obj atol.obj bitfield.obj blkclr.obj blkcpy.obj calloc.obj \
	  asmul.obj ctype_.obj getsp.obj index.obj strchr.obj \
	  inout.obj iregset.obj isalpha.obj isdigit.obj \
	  islower.obj isspace.obj isupper.obj ladd.obj land.obj \
	  linc.obj llrsh.obj longjmp.obj lor.obj lrelop.obj frelop.obj \
	  brelop.obj wrelop.obj lsub.obj lxor.obj malloc.obj idiv.obj max.obj \
	  pnum.obj ldiv.obj qsort.obj swap.obj aslr.obj bmove.obj \
	  imul.obj rand.obj alrsh.obj lmul.obj rindex.obj strrchr.obj \
	  cseek.obj shar.obj shll.obj shlr.obj strcat.obj \
	  strcmp.obj strcpy.obj strlen.obj strncat.obj strncmp.obj \
	  strncpy.obj csv.obj rcsv.obj tolower.obj \
	  toupper.obj xtoi.obj

ulibc.lib:	$(OBJS)
	-rm -f ulibc.lib
	$(LIBR) -"<" +mklib
	$(LIBR) -s ulibc.lib > ulibc.map

clean:
	rm -f ulibc.lib *.obj *.map *~ core *.\$$\$$\$$
