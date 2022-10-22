########################################################################
#
#  Use this Makefile to build the UZI180 kernel under Linux
#  using John Elliott's zxcc emulator.
#
########################################################################

VERSION = 1.6.3

CSRCS = data.c \
	devflop.c \
	devhd.c \
	devio.c \
	devmisc.c \
	devtty.c \
	filesys.c \
	machdep.c \
	process.c \
	scall1a.c \
	scall1b.c \
	scall1c.c \
	scall2.c \
	start.c

ASRCS =	flopasm.asz \
	hdasm.asz \
	machasm.asz \
	procasm.asz \
	utils.asz \
	z180asm.asz

COBJS = $(CSRCS:.c=.obj)

AOBJS = $(ASRCS:.asz=.obj)

OBJS = $(COBJS) $(AOBJS) emu.obj

all:	uzi.com

.SUFFIXES:		# delete the default suffixes
.SUFFIXES: .c .asz .obj

$(COBJS): %.obj: %.c
	zxc -o -c -v $<

emu.obj: emu.asz
	zxas -n $<

version.h:
	@echo Creating $@...
	@echo "#define VERSION   \"${VERSION}\"" > version.h
	@echo "#define BUILDDATE \""`date`"\"" >> version.h
	@echo ""

$(AOBJS): %.obj: %.asz
	zxas -j -n $<

uzi.com: version.h $(OBJS)
	zxcc link -"<" +linkuzi
	sort uzi.sym | uniq > uzi.sym.sorted

clean:
	rm -f $(OBJS) uzi.com core *~ version.h *.\$$\$$\$$
