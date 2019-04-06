/**************************************************
UZI (Unix Z80 Implementation) Utilities:  ucp.c
Modifications:
  14 June 1998 - Reformatted, restructured command
                 switch, sense Ctrl-Z in type.	HFB
  21 Sept 1999 - Corrected the 'constant expression'
                 problem, added some missing breaks.
                 HP
***************************************************/

#include <stdio.h>
#include <unix.h>
#include <config.h>
#include <extern.h>

#define UCP_VERSION  "1.1"

int16 *syserror = &udata.u_error;
static char cwd[100];
static char line[128];

#ifndef CPM
extern char *sys_errlist[];
#endif

extern void _setfdImageName(char *);

main(argc, argval)
int argc;
char *argval[];
{
    int  rdev;
    char cmd[30], arg1[30], arg2[30], arg3[30];
    int  count;
    struct filesys fsys;
    int  j, retc;
/*--    char *argv[5];--*/
    int /*gets(), */ strcmp(), _chdir(), atoi(), _mount(), _umount();

/*
    if (argc < 2)
	rdev = 0;
    else
	rdev = atoi(argval[1]);
*/
    if (argc > 1) _setfdImageName(argval[1]);
    rdev = 1;

    xfs_init(rdev);
    strcpy(cwd, "/");

    printf("UZI180 UCP version " UCP_VERSION ". Type ? for help.\n");

    for (;;) {
	printf("unix: ");
	if (gets(line) == NULL) {
	    xfs_end();
	    exit();
	}
	cmd[0] = '\0';
	*arg1 = '\0';
	arg2[0] = '\0';
	arg3[0] = '\0';
	count = sscanf(line, "%s %s %s %s", cmd, arg1, arg2, arg3);
	if (count == 0 || cmd[0] == '\0')
	    continue;

	_sync();

	if (strcmp(cmd, "\n") == 0)
	    continue;
	switch (match(cmd)) {
	case 0:			/* exit */
	    xfs_end();
	    exit();

	case 1:			/* ls */
	    if (*arg1)
		ls(arg1);
	    else
		ls(".");
	    break;

	case 2:			/* cd */
	    if (*arg1) {
		strcpy(cwd, arg1);
		if (_chdir(arg1) != 0) {
		    printf("cd: error number %d\n", *syserror);
		}
	    }
	    break;

	case 3:			/* mkdir */
	    if (*arg1)
		mkdir(arg1);
	    break;

	case 4:			/* mknod */
	    if (*arg1 && *arg2 && *arg3)
		mknod(arg1, arg2, arg3);
	    break;

	case 5:			/* chmod */
	    if (*arg1 && *arg2)
		chmod(arg1, arg2);
	    break;

	case 6:			/* get */
	    if (*arg1)
		get(arg1, 0);
	    break;

	case 7:			/* bget */
	    if (*arg1)
		get(arg1, 1);
	    break;

	case 8:			/* put */
	    if (*arg1)
		put(arg1, 0);
	    break;

	case 9:			/* bput */
	    if (*arg1)
		put(arg1, 1);
	    break;

	case 10:		/* type */
	    if (*arg1)
		type(arg1);
	    break;

	case 11:		/* dump */
	    if (*arg1)
		fdump(arg1);
	    break;

	case 12:		/* rm */
	    if (*arg1)
		unlink(arg1);
	    break;

	case 13:		/* df */
	    for (j = 0; j < 4; ++j) {
		retc = _getfsys(j, &fsys);
		if (retc == 0 && fsys.s_mounted) {
		    printf("%d:  %u blks used, %u free;  ", j,
			   (fsys.s_fsize - fsys.s_isize) - fsys.s_tfree,
			   fsys.s_tfree);
		    printf("%u inodes used, %u free\n",
			   (8 * (fsys.s_isize - 2) - fsys.s_tinode),
			   fsys.s_tinode);
		}
	    }
	    break;

	case 14:		/* rmdir */
	    if (*arg1)
		rmdir(arg1);
	    break;

	case 15:		/* mount */
	    if (*arg1 && *arg2)
		if (_mount(arg1, arg2, 0) != 0) {
		    printf("Mount error: %d\n", *syserror);
		}
	    break;

	case 16:		/* umount */
	    if (*arg1)
		if (_umount(arg1) != 0) {
		    printf("Umount error: %d\n", *syserror);
		}
	    break;

	case 50:		/* help */
	    usage();
	    break;
	    
	default:		/* ..else.. */
	    printf("Unknown command, type ? for help.\n");
	    break;
	}			/* End Switch */
    }
}


match(cmd)
char *cmd;
{
    int strcmp();

    if (strcmp(cmd, "exit") == 0)
	return (0);
    else if (strcmp(cmd, "quit") == 0)
	return (0);
    else if (strcmp(cmd, "ls") == 0)
	return (1);
    else if (strcmp(cmd, "dir") == 0)
	return (1);
    else if (strcmp(cmd, "cd") == 0)
	return (2);
    else if (strcmp(cmd, "mkdir") == 0)
	return (3);
    else if (strcmp(cmd, "mknod") == 0)
	return (4);
    else if (strcmp(cmd, "chmod") == 0)
	return (5);
    else if (strcmp(cmd, "get") == 0)
	return (6);
    else if (strcmp(cmd, "bget") == 0)
	return (7);
    else if (strcmp(cmd, "put") == 0)
	return (8);
    else if (strcmp(cmd, "bput") == 0)
	return (9);
    else if (strcmp(cmd, "type") == 0)
	return (10);
    else if (strcmp(cmd, "cat") == 0)
	return (10);
    else if (strcmp(cmd, "dump") == 0)
	return (11);
    else if (strcmp(cmd, "rm") == 0)
	return (12);
    else if (strcmp(cmd, "df") == 0)
	return (13);
    else if (strcmp(cmd, "rmdir") == 0)
	return (14);
    else if (strcmp(cmd, "mount") == 0)
	return (15);
    else if (strcmp(cmd, "umount") == 0)
	return (16);
    else if (strcmp(cmd, "help") == 0)
	return (50);
    else if (strcmp(cmd, "?") == 0)
	return (50);
    else
	return (-1);
}


usage()
{
    printf("UCP commands:\n");
    printf("?|help\n");
    printf("exit|quit\n");
    printf("dir|ls [path]\n");
    printf("cd path\n");
    printf("mkdir dirname\n");
    printf("mknod name mode dev#\n");
    printf("chmod mode path\n");
    printf("[b]get cpmfile\n");
    printf("[b]put uzifile\n");
    printf("type|cat filename\n");
    printf("dump filename\n");
    printf("rm path\n");
    printf("rmdir dirname\n");
    printf("df\n");
    printf("mount dev# path\n");
    printf("umount path\n");
}

#if 0

ls(path)
char *path;
{
    struct direct buf;
    struct stat statbuf;
    char dname[128];
    int d;
    int _open(), _read(), _stat();

    d = _open(path, 0);
    if (d < 0) {
	printf("ls: can't open %s\n", path);
	return;
    }
    while (_read(d, (char *) &buf, 16) == 16) {
	if (buf.d_name[0] == '\0')
	    continue;

	if (path[0] != '.' || path[1]) {
	    strcpy(dname, path);
	    strcat(dname, "/");
	} else
	    dname[0] = '\0';
	strcat(dname, buf.d_name);

	if (_stat(dname, &statbuf) != 0) {
	    printf("ls: can't stat %s\n", dname);
	    continue;
	}
	if ((statbuf.st_mode & F_MASK) == F_DIR)
	    strcat(dname, "/");

	printf("%-6d %-15s",
	       (statbuf.st_mode & F_CDEV) ?
	       statbuf.st_rdev :
	       512 * statbuf.st_size.o_blkno + statbuf.st_size.o_offset,
	       dname);

	printf("  0%-6o %-2d %-5d\n", statbuf.st_mode, statbuf.st_nlink,
	       statbuf.st_ino);
    }
    _close(d);
}

#else

prmode(mode)
int mode;
{
    if (mode & 4)
	printf("r");
    else
	printf("-");

    if (mode & 2)
	printf("w");
    else
	printf("-");

    if (mode & 1)
	printf("x");
    else
	printf("-");
}

char *month[] =
 { "Jan", "Feb", "Mar", "Apr",
   "May", "Jun", "Jul", "Aug",
   "Sep", "Oct", "Nov", "Dec" };

ls(path)
char *path;
{
    struct direct buf;
    struct stat statbuf;
    char dname[128];
    int d, st, year;
    int _open(), _read(), _stat();

/*
   if (_stat(path, &statbuf) != 0 || (statbuf.st_mode & F_MASK) != F_DIR) {
   printf("ls: can't stat %s\n", path);
   return -1;
   }
 */

    d = _open(path, 0);
    if (d < 0) {
	printf("ls: can't open %s\n", path);
	return -1;
    }
    while (_read(d, (char *) &buf, 16) == 16) {
	if (buf.d_name[0] == '\0')
	    continue;

	if (path[0] != '.' || path[1]) {
	    strcpy(dname, path);
	    strcat(dname, "/");
	} else {
	    dname[0] = '\0';
	}

	strcat(dname, buf.d_name);

	if (_stat(dname, &statbuf) != 0) {
	    printf("ls: can't stat %s\n", dname);
	    break;
	}
	st = (statbuf.st_mode & F_MASK);

	if ((st & F_MASK) == F_DIR)	/* & F_MASK is redundant */
	    printf("d");
	else if ((st & F_MASK) == F_CDEV)
	    printf("c");
	else if ((st & F_MASK) == F_BDEV)
	    printf("b");
	else if ((st & F_MASK) == F_PIPE)
	    printf("p");
	else if ((st & F_REG) == 0)
	    printf("l");
	else
	    printf("-");

	prmode(statbuf.st_mode >> 6);
	prmode(statbuf.st_mode >> 3);
	prmode(statbuf.st_mode);

	printf("%4d %5d", statbuf.st_nlink, statbuf.st_ino);

	if ((statbuf.st_mode & F_MASK) == F_DIR)
	    strcat(dname, "/");

	printf("%12u ",
	       (statbuf.st_mode & F_CDEV) ?
	       statbuf.st_rdev :
	       512 * statbuf.st_size.o_blkno + statbuf.st_size.o_offset);

	if (statbuf.st_mtime.t_time == 0) {	/* st_mtime? */
	    /*printf("--- -- ----   --:--");*/
	    printf("                   ");
	} else {
	    year = ((statbuf.st_mtime.t_date >> 9) & 0x7F) + 1900;
	    if (year < 1978)
		year += 100;

	    printf("%s %02d %4d   ",
		   month[((statbuf.st_mtime.t_date >> 5) & 0x0F) - 1],
		   (statbuf.st_mtime.t_date & 0x1F),
		   year);

	    printf("%2d:%02d",
		   ((statbuf.st_mtime.t_time >> 11) & 0x1F),
		   ((statbuf.st_mtime.t_time >> 5) & 0x3F)/*,
		   (statbuf.st_mtime.t_time & 0x1F) */ );
	}

	printf("  %-15s\n", dname);
    }
    _close(d);
}

#endif


chmod(path, modes)
char *path;
char *modes;
{
    int mode;
    int _chmod();

    mode = -1;
    sscanf(modes, "%o", &mode);
    if (mode == -1) {
	printf("chmod: bad mode\n");
	return (-1);
    }
    if (_chmod(path, mode)) {
	printf("_chmod: error %d\n", *syserror);
	return (-1);
    }
}


mknod(path, modes, devs)
char *path;
char *modes;
char *devs;
{
    int mode;
    int dev;
    int _mknod();

    mode = -1;
    sscanf(modes, "%o", &mode);
    if (mode == -1) {
	printf("mknod: bad mode\n");
	return (-1);
    }
    if ((mode & F_MASK) != F_BDEV && (mode & F_MASK) != F_CDEV) {
	printf("mknod: mode is not device\n");
	return (-1);
    }
    dev = -1;
    sscanf(devs, "%d", &dev);
    if (dev == -1) {
	printf("mknod: bad device\n");
	return (-1);
    }
    if (_mknod(path, mode, dev) != 0) {
	printf("_mknod: error %d\n", *syserror);
	return (-1);
    }
    return (0);
}



mkdir(path)
char *path;
{
    char dot[100];
    int _mknod(), _link();

    if (_mknod(path, 040000 | 0777, 0) != 0) {
	printf("mkdir: mknod error %d\n", *syserror);
	return (-1);
    }
    strcpy(dot, path);
    strcat(dot, "/.");
    if (_link(path, dot) != 0) {
	printf("mkdir: link \".\" error %d\n", *syserror);
	return (-1);
    }
    strcpy(dot, path);
    strcat(dot, "/..");
    if (_link(".", dot) != 0) {
	printf("mkdir: link \"..\" error %d\n", *syserror);
	return (-1);
    }
    return (0);
}



get(arg, binflag)
char *arg;
int binflag;
{
    FILE *fp;
    int d;
    char cbuf[512];
    int nread;
    /*FILE *fopen(); */
    int _creat() /*, fread(), _write() */ ;

    fp = fopen(arg, binflag ? "rb" : "r");
    if (fp == NULL) {
	printf("Source file not found\n");
	return (-1);
    }
    d = _creat(arg, 0666);
    if (d < 0) {
	printf("Cant open unix file error %d\n", *syserror);
	return (-1);
    }
    for (;;) {
	nread = fread(cbuf, 1, 512, fp);
	if (nread == 0)
	    break;
	if (_write(d, cbuf, nread) != nread) {
	    printf("_write error %d\n", *syserror);
	    fclose(fp);
	    _close(d);
	    return (-1);
	}
    }
    fclose(fp);
    _close(d);
    _sync();
    return (0);
}


put(arg, binflag)
char *arg;
int binflag;
{
    FILE *fp;
    int d;
    char cbuf[512];
    int nread;
    /*FILE *fopen(); */
    int _open() /*, fwrite(), _read() */ ;

    fp = fopen(arg, binflag ? "wb" : "w");
    if (fp == NULL) {
	printf("Cant open destination file.\n");
	return (-1);
    }
    d = _open(arg, 0);
    if (d < 0) {
	printf("Cant open unix file error %d\n", *syserror);
	return (-1);
    }
    for (;;) {
	if ((nread = _read(d, cbuf, 512)) == 0)
	    break;
	if (fwrite(cbuf, 1, nread, fp) != nread) {
	    printf("fwrite error");
	    fclose(fp);
	    _close(d);
	    return (-1);
	}
    }
    fclose(fp);
    _close(d);
    return (0);
}


type(arg)
char *arg;
{
    int d, i;
    char cbuf[512];
    int nread;
    int _open(), _read();

    d = _open(arg, 0);
    if (d < 0) {
	printf("Cant open unix file error %d\n", *syserror);
	return (-1);
    }
    for (;;) {
	if ((nread = _read(d, cbuf, 512)) == 0)
	    break;

	for (i = 0; i < nread; i++) {
	    if (cbuf[i] == 0x1a)
		break;
	    fputc(cbuf[i], stdout);
	}
    }
    fputc('\n', stdout);
    _close(d);
    return (0);
}


fdump(arg)
char *arg;
{
    int d;
    char cbuf[512];
    int nread;
    int _open(), _read();

    printf("Dump starting.\n");
    d = _open(arg, 0);
    if (d < 0) {
	printf("Cant open unix file error %d\n", *syserror);
	return (-1);
    }
    for (;;) {
	if ((nread = _read(d, cbuf, 512)) == 0)
	    break;
    }
    _close(d);
    printf("Dump done.\n");
    return (0);
}


unlink(path)
char *path;
{
    struct stat statbuf;
    int _stat(), _unlink();

    if (_stat(path, &statbuf) != 0) {
	printf("unlink: can't stat %s\n", path);
	return (-1);
    }
    if ((statbuf.st_mode & F_MASK) == F_DIR) {
	printf("unlink: %s is a directory\n", path);
	return (-1);
    }
    if (_unlink(path) != 0) {
	printf("unlink: _unlink errn=or %d\n", *syserror);
	return (-1);
    }
    return (0);
}


rmdir(path)
char *path;
{
    struct stat statbuf;
    char newpath[100];
    struct direct dir;
    int fd;
    int _stat(), _open(), _read(), strcmp(), _unlink();

    if (_stat(path, &statbuf) != 0) {
	printf("rmdir: can't stat %s\n", path);
	return (-1);
    }
    if ((statbuf.st_mode & F_DIR) == 0) {
/*-- Constant expression !!!  HFB --*/
	printf("rmdir: %s is not a directory\n", path);
	return (-1);
    }
    if ((fd = _open(path, 0)) < 0) {
	printf("rmdir: %s is unreadable\n", path);
	return (-1);
    }
    while (_read(fd, (char *) &dir, sizeof(dir)) == sizeof(dir)) {
	if (dir.d_ino == 0)
	    continue;
	if (!strcmp(dir.d_name, ".") || !strcmp(dir.d_name, ".."))
	    continue;
	printf("rmdir: %s is not empty\n", path);
	_close(fd);
	return (-1);
    }
    _close(fd);

    strcpy(newpath, path);
    strcat(newpath, "/.");
    if (_unlink(newpath) != 0) {
	printf("rmdir: can't unlink \".\"  error %d\n", *syserror);
	/*return (-1); */
    }
    strcat(newpath, ".");
    if (_unlink(newpath) != 0) {
	printf("rmdir: can't unlink \"..\"  error %d\n", *syserror);
	/*return (-1); */
    }
    if (_unlink(path) != 0) {
	printf("rmdir: _unlink error %d\n", *syserror);
	return (-1);
    }
    return (0);
}
