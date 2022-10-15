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


/* Malloc: a certified malloc */
char *Malloc(int n)
{
    char *m;

    if ((m = (char *) malloc(n)) == (char *) NULL)
	fatal("out of memory", "");
    return m;
}

/* Salloc: allocate space for a string */
char *Salloc(char *s)
{
    return strcpy(Malloc(strlen(s) + 1), s);
}


/*######################## DIAGNOSTICS ##############################*/

void nonfatal(char *s1, char *s2)
{
    fprintf(stderr, "%s: %s%s\n", prog, s1, s2);
}

void fatal(char *s1, char *s2)
{
    nonfatal(s1, s2);
    exit(1);
}

/*################### SMATCH #########################*/
/* Don't try to understand the following one... */
int smatch(char *s, char *t) /* shell-like matching */
{
    register n;

    if (*t == '\0')
	return *s == '\0';
    if (*t == '*') {
	++t;
	do
	    if (smatch(s, t))
		return 1;
	while (*s++ != '\0');
	return 0;
    }
    if (*s == '\0')
	return 0;
    if (*t == '\\')
	return (*s == *++t) ? smatch(++s, ++t) : 0;
    if (*t == '?')
	return smatch(++s, ++t);
    if (*t == '[') {
	while (*++t != ']') {
	    if (*t == '\\')
		++t;
	    if (*(t + 1) != '-')
		if (*t == *s) {
		    while (*++t != ']')
			if (*t == '\\')
			    ++t;
		    return smatch(++s, ++t);
		} else
		    continue;
	    if (*(t + 2) == ']')
		return (*s == *t || *s == '-');
	    n = (*(t + 2) == '\\') ? 3 : 2;
	    if (*s >= *t && *s <= *(t + n)) {
		while (*++t != ']')
		    if (*t == '\\')
			++t;
		return smatch(++s, ++t);
	    }
	    t += n;
	}
	return 0;
    }
    return (*s == *t) ? smatch(++s, ++t) : 0;
}
