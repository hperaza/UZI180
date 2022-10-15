.SUFFIXES:	.obj .asz

AS	= zxas
ASFLAGS	= -j -n

.asz.obj:
	$(AS) $(ASFLAGS) $<

SRCS	= crt.asz

OBJS	= crt.obj

all:	$(OBJS)

clean:
	rm -f *.obj *~ core *.\$$\$$\$$
