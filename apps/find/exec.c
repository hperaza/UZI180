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

extern char **environ;

#define execv(path, args) execve(path, args, environ);

/*####################### EXECUTE ###########################*/
/* Do -exec or -ok */

char *find_bin(char *s)
{
    char *f, *l, buf[PATH_MAX];

    if (*s == '/')		/* absolute path name */
	return ((access(s, 1) == 0) ? s : (char *) 0);
    l = f = epath;
    for (;;) {
	if (*l == ':' || *l == 0) {
	    if (l == f) {
		if (access(s, 1) == 0)
		    return Salloc(s);
		f++;
	    } else {
		register char *p = buf, *q = s;

		while (f != l)
		    *p++ = *f++;
		f++;
		*p++ = '/';
		while (*p++ = *q++) {
		}
		if (access(buf, 1) == 0)
		    return Salloc(buf);
	    }
	    if (*l == 0)
		break;
	}
	l++;
    }
    return 0;
}

int execute(int op, struct exec *e, char *path)
{
    int  s, pid;
    register char **p, **q;
    char *argv[MAXARG];

    for (p = e->e_vec, q = argv; *p;)	/* replace the {}s */
	if ((*q++ = *p++) == (char *) -1)
	    q[-1] = path;
    *q = '\0';
    if (op == OP_OK) {
	char answer[10];

	for (p = &argv[2]; *p; p++) {
	    write(tty, *p, strlen(*p));
	    write(tty, " ", 1);
	}
	write(tty, "? ", 2);
	if (read(tty, answer, 10) < 2 || *answer != 'y')
	    return 0;
    }
    if ((pid = fork()) == -1)
	fatal("can't fork", "");
    if (pid == 0) {
	register i = 3;

	while (close(i++) == 0) {
	}			/* wow!!! */
	execv(argv[1], &argv[2]);	/* binary itself? */
	execv(argv[0], &argv[1]);	/* shell command? */
	fatal("exec failure: ", argv[1]);	/* none of them! */
	exit(127);
    }
    return ((wait(&s) == pid) && (s == 0));
}
