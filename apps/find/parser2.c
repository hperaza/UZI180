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


void domode(int op, int *mode, int bits)
{
    switch (op) {
    case '-':
	*mode &= ~bits;
	break;			/* clear bits */
    case '=':
	*mode |= bits;
	break;			/* set bits */
    case '+':
	*mode |= (bits & ~um);	/* set, but take umask into account */
    }
}

void fmode(char *str, long *pl, int *ps)
{
    int m = 0, w, op;
    char *p = str;

    if (*p == '-') {
	*ps = -1;
	p++;
    } else
	*ps = 0;

    while (*p) {
	w = 0;
	if (ISOPER(*p))
	    w = MUSER | MGROUP | MOTHERS;
	else if (!ISWHO(*p))
	    fatal("u, g, o, or a expected: ", p);
	else {
	    while (ISWHO(*p)) {
		switch (*p) {
		case 'u':
		    w |= MUSER;
		    break;
		case 'g':
		    w |= MGROUP;
		    break;
		case 'o':
		    w |= MOTHERS;
		    break;
		case 'a':
		    w = MUSER | MGROUP | MOTHERS;
		}
		p++;
	    }
	    if (!ISOPER(*p))
		fatal("-, + or = expected: ", p);
	}
	op = *p++;
	while (ISMODE(*p)) {
	    switch (*p) {
	    case 'r':
		if (w & MUSER)
		    domode(op, &m, S_IRUSR);
		if (w & MGROUP)
		    domode(op, &m, S_IRGRP);
		if (w & MOTHERS)
		    domode(op, &m, S_IROTH);
		break;
	    case 'w':
		if (w & MUSER)
		    domode(op, &m, S_IWUSR);
		if (w & MGROUP)
		    domode(op, &m, S_IWGRP);
		if (w & MOTHERS)
		    domode(op, &m, S_IWOTH);
		break;
	    case 'x':
		if (w & MUSER)
		    domode(op, &m, S_IXUSR);
		if (w & MGROUP)
		    domode(op, &m, S_IXGRP);
		if (w & MOTHERS)
		    domode(op, &m, S_IXOTH);
		break;
	    case 's':
		if (w & MUSER)
		    domode(op, &m, S_ISUID);
		if (w & MGROUP)
		    domode(op, &m, S_ISGID);
		break;
	    case 't':
		domode(op, &m, S_ISVTX);
	    }
	    p++;
	}
	if (*p) {
	    if (*p == ',')
		p++;
	    else
		fatal("garbage at end of mode string: ", p);
	}
    }
    *pl = m;
}
