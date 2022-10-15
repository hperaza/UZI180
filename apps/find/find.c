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

void find(char *path, struct node *pred, char *last)
{
    register char *send, *p;
    struct stat st;
    DIR *dp;
    struct direct *de;
    char spath[PATH_MAX];

    send = spath;

    if (path[1] == '\0' && *path == '/') {
	*send++ = '/';
	*send = '\0';
    } else {
	while (*send++ = *path++) {
	}
    }

    if (LSTAT(spath, &st) == -1) {
	nonfatal("can't get status of ", spath);
    } else {
	switch (xdev_flag) {
	case 0:
	    break;
	case 1:
	    if (st.st_dev != devnr)
		return;
	    break;
	case 2:		/* set current device number */
	    xdev_flag = 1;
	    devnr = st.st_dev;
	    break;
	}

	prune_here = 0;
	if (!depth_flag && check(spath, &st, pred, last) && needprint)
	    printf("%s\n", spath);
	if (!prune_here && (st.st_mode & S_IFMT) == S_IFDIR) {
	    if ((dp = opendir(spath)) == NULL) {
		nonfatal("can't read directory ", spath);
		return;
	    }
	    send[-1] = '/';
	    while ((de = readdir(dp)) != NULL) {
		p = de->d_name;
		if ((de->d_name[0] != '.') || ((de->d_name[1])
					       && ((de->d_name[1] != '.')
						   || (de->d_name[2])))) {
		    strcpy(send, de->d_name);
		    find(spath, pred, send);
		}
	    }
	    closedir(dp);
	}
	if (depth_flag) {
	    send[-1] = '\0';
	    if (check(spath, &st, pred, last) && needprint)
		printf("%s\n", spath);
	}
    }
}

int lex(char *str)
{
    if (str == (char *) NULL)
	return EOI;
    if (*str == '-') {
	register struct oper *op;

	str++;
	for (op = ops; op->op_str; op++)
	    if (strcmp(str, op->op_str) == 0)
		break;
	return op->op_val;
    }
    if (str[1] == 0) {
	switch (*str) {
	case '(':
	    return LPAR;
	case ')':
	    return RPAR;
	case '!':
	    return NOT;
	}
    }
    return NONE;
}

struct node *newnode(int t)
{
    struct node *n = (struct node *) Malloc(sizeof(struct node));

    n->n_type = t;
    return n;
}
