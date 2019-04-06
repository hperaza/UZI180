/***************************************************************
   UZI (Unix Z80 Implementation) Kernel:  data.c
----------------------------------------------------------------
 Adapted from UZI By Doug Braun, and UZI280 by Stefan Nitschke
            Copyright (C) 1998 by Harold F. Bower
       Portions Copyright (C) 1995 by Stefan Nitschke
****************************************************************/
/* Revisions:
 */

#include <unix.h>
#include <config.h>
#define MAIN

#include <extern.h>         /* This declares all the tables, etc */

int stdin, stdout, stderr;  /* Necessary for library, but never referenced */

/* Dispatch table for system calls */
extern int
        __exit(),
        _open(),
        _close(),
        _creat(),
        _mknod(),
        _link(),
        _unlink(),
        _read(),
        _write(),
        _seek(),
        _chdir(),
        _sync(),
        _access(),
        _chmod(),
        _chown(),
        _stat(),
        _fstat(),
        _dup(),
        _getpid(),
        _getppid(),
        _getuid(),
        _umask(),
        _getfsys(),
        _execve(),
        _wait(),
        _setuid(),
        _setgid(),
        _time(),
        _stime(),
        _ioctl(),
        _brk(),
        _sbrk(),
        _fork(),
        _mount(),
        _umount(),
        _signal(),
        _dup2(),
        _pause(),
        _alarm(),
        _kill(),
        _pipe(),
        _getgid(),
        _times(),
        _utime(),
        _geteuid(),
        _getegid();

int (*disp_tab[])() =
{       __exit,
        _open,
        _close,
        _creat,
        _mknod,
        _link,
        _unlink,
        _read,
        _write,
        _seek,
        _chdir,
        _sync,
        _access,
        _chmod,
        _chown,
        _stat,
        _fstat,
        _dup,
        _getpid,
        _getppid,
        _getuid,
        _umask,
        _getfsys,
        _execve,
        _wait,
        _setuid,
        _setgid,
        _time,
        _stime,
        _ioctl,
        _brk,
        _sbrk,
        _fork,
        _mount,
        _umount,
        _signal,
        _dup2,
        _pause,
        _alarm,
        _kill,
        _pipe,
        _getgid,
        _times,
        _utime,
        _geteuid,
        _getegid
};

int ncalls = sizeof (disp_tab) / sizeof (int(*)());
