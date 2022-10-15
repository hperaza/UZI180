/* find - look for files satisfying a predicate       Author: E. Baalbergen */
/* Original author: Erik Baalbergen; POSIX compliant version: Bert Laverman */
/* UZI180 port by Hector Peraza */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#if 0
#include <time.h>
#endif
#include <pwd.h>
#include <grp.h>

#include "find.h"

static void opentty() {
    char *ttyname, *getenv();
    
    ttyname = getenv("CTTY");
    if (ttyname == NULL) ttyname = "/dev/tty";
    tty = open(ttyname, O_RDWR);
    if (tty < 0)
	fatal("can't open ", ttyname);
}

struct node *simple(int t)
{
    struct node *p = newnode(t);
    struct exec *e;
    struct stat est;
    struct passwd *pw;
    struct group *gr;
    long l;
    int i;

    switch (t) {
    case OP_TYPE:
	checkarg(*++ipp);
	switch (**ipp) {
	case 'b':
	    p->n_info.n_int.n_val = S_IFBLK;
	    break;
	case 'c':
	    p->n_info.n_int.n_val = S_IFCHR;
	    break;
	case 'd':
	    p->n_info.n_int.n_val = S_IFDIR;
	    break;
	case 'f':
	    p->n_info.n_int.n_val = S_IFREG;
	    break;
	case 'l':
#ifdef S_IFLNK
	    p->n_info.n_int.n_val = S_IFLNK;
#else
	    p->n_info.n_int.n_val = ~0;	/* Always unequal. */
#endif
	    break;
	default:
	    fatal("-type needs b, c, d, f or l", "");
	}
	break;
    case OP_USER:
	checkarg(*++ipp);
	if (((pw = getpwnam(*ipp)) == NULL)
	    && isnumber(*ipp, 10, 0))
	    number(*ipp, 10, &(p->n_info.n_int.n_val),
		   &(p->n_info.n_int.n_sign));
	else {
	    if (pw == NULL)
		fatal("unknown user: ", *ipp);
	    p->n_info.n_int.n_val = pw->pw_uid;
	    p->n_info.n_int.n_sign = 0;
	}
	break;
    case OP_GROUP:
	checkarg(*++ipp);
	if (((gr = getgrnam(*ipp)) == NULL)
	    && isnumber(*ipp, 10, 0))
	    number(*ipp, 10, &(p->n_info.n_int.n_val),
		   &(p->n_info.n_int.n_sign));
	else {
	    if (gr == NULL)
		fatal("unknown group: ", *ipp);
	    p->n_info.n_int.n_val = gr->gr_gid;
	    p->n_info.n_int.n_sign = 0;
	}
	break;
    case OP_SIZE:
	checkarg(*++ipp);
	i = strlen(*ipp) - 1;
	if ((*ipp)[i] == 'c') {
	    p->n_type = OP_SIZEC;	/* Count in bytes i.s.o. blocks */
	    (*ipp)[i] = '\0';
	}
	number(*ipp, 10, &(p->n_info.n_int.n_val),
	       &(p->n_info.n_int.n_sign));
	break;
    case OP_LINKS:
    case OP_INUM:
	checkarg(*++ipp);
	number(*ipp, 10, &(p->n_info.n_int.n_val),
	       &(p->n_info.n_int.n_sign));
	break;
    case OP_PERM:
	checkarg(*++ipp);
	if (isnumber(*ipp, 8, 1))
	    number(*ipp, 8, &(p->n_info.n_int.n_val),
		   &(p->n_info.n_int.n_sign));
	else
	    fmode(*ipp, &(p->n_info.n_int.n_val),
		  &(p->n_info.n_int.n_sign));
	break;
    case OP_ATIME:
    case OP_CTIME:
    case OP_MTIME:
	checkarg(*++ipp);
	number(*ipp, 10, &l, &(p->n_info.n_int.n_sign));
	p->n_info.n_int.n_val = current_time - l * SECS_PER_DAY;
	/* More than n days old means less than the absolute time */
	p->n_info.n_int.n_sign *= -1;
	break;
    case OP_EXEC:
    case OP_OK:
	checkarg(*++ipp);
	e = (struct exec *) Malloc(sizeof(struct exec));
	e->e_cnt = 2;
	e->e_vec[0] = SHELL;
	p->n_info.n_exec = e;
	while (*ipp) {
	    if (**ipp == ';' && (*ipp)[1] == '\0') {
		e->e_vec[e->e_cnt] = 0;
		break;
	    }
	    e->e_vec[(e->e_cnt)++] =
		(**ipp == '{' && (*ipp)[1] == '}'
		 && (*ipp)[2] == '\0') ? (char *) (-1) : *ipp;
	    ipp++;
	}
	if (*ipp == 0)
	    fatal("-exec/-ok: ; missing", "");
	if ((e->e_vec[1] = find_bin(e->e_vec[2])) == 0)
	    fatal("can't find program ", e->e_vec[2]);
	if (t == OP_OK)
	    opentty();
	break;
    case OP_NEWER:
	checkarg(*++ipp);
	if (LSTAT(*ipp, &est) == -1)
	    fatal("-newer: can't get status of ", *ipp);
	p->n_info.n_int.n_val = convuzitime(&est.st_mtime);
	break;
    case OP_NAME:
	checkarg(*++ipp);
	p->n_info.n_str = *ipp;
	break;
    case OP_XDEV:
	xdev_flag = 1;
	break;
    case OP_DEPTH:
	depth_flag = 1;
	break;
    case OP_PRUNE:
    case OP_PRINT:
    case OP_PRINT0:
    case OP_NOUSER:
    case OP_NOGROUP:
	break;
    default:
	fatal("syntax error, operator expected", "");
    }
    if ((t == OP_PRINT) || (t == OP_PRINT0) || (t == OP_EXEC)
	|| (t == OP_OK)) needprint = 0;

    return p;
}
