###########################################################################
#
#  Makefile to build the set of UZI180 simple utilities under Linux
#  using John Elliott's zxcc CP/M emulator and HiTech C compiler.
#
###########################################################################


.SUFFIXES: .c .obj

SRCS  = basename.c \
	bd.c \
	cal.c \
	cat.c \
	chgrp.c \
	chmod.c \
	chown.c \
	cp.c \
	cut.c \
	date.c \
	dd.c \
	df.c \
	dirname.c \
	du.c \
	dugraph.c \
	echo.c \
	ed.c \
	expr.c \
	factor.c \
	false.c \
	fsck.c \
	grep.c \
	id.c \
	init.c \
	kill.c \
	ll.c \
	ln.c \
	ls.c \
	mkdir.c \
	mkfs.c \
	mknod.c \
	more.c \
	mount.c \
	mv.c \
	od.c \
	passwd.c \
	patchcpm.c \
	patchold.c \
	printenv.c \
	printf.c \
	prtroot.c \
	ps.c \
	pwd.c \
	rm.c \
	rmdir.c \
	sleep.c \
	ssh.c \
	strings.c \
	su.c \
	sync.c \
	termcap.c \
	tget.c \
	touch.c \
	tr.c \
	tree.c \
	true.c \
	umount.c \
	uniq.c \
	uud.c \
	uue.c \
	uzidate.c \
	wc.c \
	which.c \
	whoami.c

OBJS = $(SRCS:.c=.obj)

LIBS = uzilibc.lib

APPS = $(OBJS:.obj=)

all: $(APPS)

$(OBJS): $(SRCS)

.c.obj:
	zxc -o -c -v $<

printf: printf.obj
	zxlink -Z -W3 -Ptext=0,data,bss -C100H -o$@ crt.obj $< libf.lib $(LIBS)

tget: tget.obj
	zxlink -Z -W3 -Ptext=0,data,bss -C100H -o$@ crt.obj $< libtcap.lib $(LIBS)

termcap: termcap.obj
	zxlink -Z -W3 -Ptext=0,data,bss -C100H -o$@ crt.obj $< libtcap.lib $(LIBS)

%: %.obj
	zxlink -Z -W3 -Ptext=0,data,bss -C100H -o$@ crt.obj $< $(LIBS)

clean:
	rm -f $(OBJS) $(APPS) core *~ *.\$$\$$\$$

rmbak:
	rm -f *~ core *.\$$\$$\$$
