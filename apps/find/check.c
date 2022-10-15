/* find - look for files satisfying a predicate       Author: E. Baalbergen */
/* Original author: Erik Baalbergen; POSIX compliant version: Bert Laverman */
/* UZI180 port by Hector Peraza */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#if 0
#include <time.h>
#endif
#include <pwd.h>
#include <grp.h>


#include "find.h"

int check(char *path, struct stat *st, struct node *n, char *last)
{
    long fsize, mtime, atime, ctime;

    fsize = st->st_size.o_blkno * 512L + st->st_size.o_offset;
    mtime = convuzitime(&st->st_mtime);
    atime = convuzitime(&st->st_atime);
    ctime = convuzitime(&st->st_ctime);

    if (n == (struct node *) NULL)
	return 1;

    switch (n->n_type) {
    case OP_AND:
	return check(path, st, n->n_info.n_opnd.n_left, last) &&
 	       check(path, st, n->n_info.n_opnd.n_right, last);
    case OP_OR:
	return check(path, st, n->n_info.n_opnd.n_left, last) ||
	       check(path, st, n->n_info.n_opnd.n_right, last);
    case NOT:
	return !check(path, st, n->n_info.n_opnd.n_left, last);
    case OP_NAME:
	return smatch(last, n->n_info.n_str);
    case OP_PERM:
	if (n->n_info.n_int.n_sign < 0)
	    return (st->st_mode & (int) n->n_info.n_int.n_val) ==
		(int) n->n_info.n_int.n_val;
	return (st->st_mode & 07777) == (int) n->n_info.n_int.n_val;
    case OP_NEWER:
	return mtime > n->n_info.n_int.n_val;
    case OP_TYPE:
	return (st->st_mode & S_IFMT) == n->n_info.n_int.n_val;
    case OP_LINKS:
	return ichk((long) (st->st_nlink), n);
    case OP_USER:
	return st->st_uid == n->n_info.n_int.n_val;
    case OP_GROUP:
	return st->st_gid == n->n_info.n_int.n_val;
    case OP_SIZE:
	return ichk((fsize == 0L) ? 0L :
		    (long) ((fsize - 1) / BSIZE + 1), n);
    case OP_SIZEC:
	return ichk(fsize, n);
    case OP_INUM:
	return ichk((long) (st->st_ino), n);
    case OP_ATIME:
	return ichk(atime, n);
    case OP_CTIME:
	return ichk(ctime, n);
    case OP_MTIME:
	return ichk(mtime, n);
    case OP_EXEC:
    case OP_OK:
	return execute(n->n_type, n->n_info.n_exec, path);
    case OP_PRINT:
	printf("%s\n", path);
	return 1;
    case OP_PRINT0:
	printf("%s", path);
	putchar(0);
	return 1;
    case OP_XDEV:
    case OP_DEPTH:
	return 1;
    case OP_PRUNE:
	prune_here = 1;
	return 1;
    case OP_NOUSER:
	return (getpwuid(st->st_uid) == (struct passwd *) NULL);
    case OP_NOGROUP:
	return (getgrgid(st->st_gid) == (struct group *) NULL);
    }
    fatal("ILLEGAL NODE", "");
    return 0;			/* Never reached */
}

int ichk(long val, struct node *n)
{
    switch (n->n_info.n_int.n_sign) {
    case 0:
	return val == n->n_info.n_int.n_val;
    case 1:
	return val > n->n_info.n_int.n_val;
    case -1:
	return val < n->n_info.n_int.n_val;
    }
    fatal("internal: bad n_sign", "");
    return 0;			/* Never reached */
}
