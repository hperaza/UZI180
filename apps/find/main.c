/* find - look for files satisfying a predicate       Author: E. Baalbergen */
/* Original author: Erik Baalbergen; POSIX compliant version: Bert Laverman */
/* UZI180 port by Hector Peraza */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#if 0
#include <time.h>
#endif
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <limits.h>

#include "find.h"

/*extern long convuzitime(time_t *tm);*/

struct oper ops[] = {
    { "name",    OP_NAME    },
    { "perm",    OP_PERM    },
    { "type",    OP_TYPE    },
    { "links",   OP_LINKS   },
    { "user",    OP_USER    },
    { "group",   OP_GROUP   },
    { "size",    OP_SIZE    },
    { "inum",    OP_INUM    },
    { "atime",   OP_ATIME   },
    { "ctime",   OP_CTIME   },
    { "mtime",   OP_MTIME   },
    { "exec",    OP_EXEC    },
    { "ok",      OP_OK      },
    { "print",   OP_PRINT   },
    { "print0",  OP_PRINT0  },
    { "newer",   OP_NEWER   },
    { "a",       OP_AND     }, 
    { "o",       OP_OR      },
    { "xdev",    OP_XDEV    },
    { "depth",   OP_DEPTH   },
    { "prune",   OP_PRUNE   },
    { "nouser",  OP_NOUSER  },
    { "nogroup", OP_NOGROUP },
    { 0,         0          }
};

char **ipp;			/* pointer to next argument during parsing       */
char *prog;			/* program name (== argv [0])                    */
char *epath;			/* value of PATH environment string              */
long current_time;		/* for computing age                             */
int tty;			/* fd for /dev/tty when using -ok                */
int xdev_flag = 0;		/* cross device boundaries?                      */
int devnr;			/* device nr of first inode                      */
int depth_flag = 0;		/* descend before check?                         */
int prune_here;			/* This is Baaaad! Don't ever do this again!     */
int um;				/* current umask()                               */
int needprint = 1;		/* implicit -print needed?                       */

int time_zone = 0;


/* Main: the main body */
int main(int argc, char *argv[])
{
    char **pathlist, *path, *last;
    int pathcnt = 0, i;
    struct node *pred;
    time_t uzi_time;

    prog = *argv++;		/* set program name (for diagnostics)    */
    if ((epath = getenv("PATH")) == (char *) NULL)
	fatal("Can't get path from environment", "");
    umask(um = umask(0));	/* non-destructive get-umask :-)         */
    time(&uzi_time);		/* get current time                      */
    current_time = convuzitime(&uzi_time);

    pathlist = argv;
    while (--argc > 0 && lex(*argv) == NONE) {	/* find paths            */
	pathcnt++;
	argv++;
    }
    if (pathcnt == 0)		/* there must be at least one path       */
	fatal("Usage: path-list [predicate-list]", "");

    ipp = argv;			/* prepare for parsing                   */
    if (argc != 0) {		/* If there is anything to parse,        */
	pred = expr(lex(*ipp));	/* then do so                            */
	if (lex(*++ipp) != EOI)	/* Make sure there's nothing left        */
	    fatal("syntax error: garbage at end of predicate", "");
    } else {			/* No predicate list                     */
	pred = (struct node *) NULL;
    }

    for (i = 0; i < pathcnt; i++) {
	if (xdev_flag)
	    xdev_flag = 2;
	path = pathlist[i];
	if ((last = strrchr(path, '/')) == NULL)
	    last = path;
	else
	    last++;
	find(path, pred, last);
    }
    return 0;
}
