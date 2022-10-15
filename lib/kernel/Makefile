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

OBJS	= access.obj alarm.obj brk.obj chdir.obj chmod.obj chown.obj \
	  close.obj creat.obj dup.obj dup2.obj execve.obj _exit.obj fork.obj \
	  fstat.obj getegid.obj geteuid.obj getfsys.obj getgid.obj getpid.obj \
	  getppid.obj getuid.obj ioctl.obj kcall.obj kill.obj link.obj \
	  mknod.obj mount.obj open.obj pause.obj pipe.obj read.obj sbrk.obj \
	  seek.obj setgid.obj setuid.obj signal.obj stat.obj \
	  stime.obj sync.obj time.obj times.obj umask.obj umount.obj \
	  unlink.obj utime.obj wait.obj write.obj

SRCS	= access.as alarm.as brk.as chdir.as chmod.as chown.as close.as \
	  creat.as dup.as dup2.as execve.as _exit.as fork.as fstat.as \
	  getegid.as geteuid.as getfsys.as getgid.as getpid.as getppid.as \
	  getuid.as ioctl.as kcall.as kill.as link.as mknod.as mount.as \
	  open.as pause.as pipe.as read.as sbrk.as seek.as setgid.as \
	  setuid.as signal.as stat.as stime.as sync.as time.as times.as \
	  umask.as umount.as unlink.as utime.as wait.as write.as

LIB	= ukrnl.lib

all:	$(LIB)

$(LIB):	$(OBJS)
	-rm -f $@
	$(LIBR) -"<" +mklib
	$(LIBR) -s $@ > ukrnl.map

clean:
	rm -f $(LIB) *.obj *.map *~ core *.\$$\$$\$$
