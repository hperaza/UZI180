#include <setjmp.h>
#include <unix.h>
#include "sh.h"

/*
 * Is any character from s1 in s2?
 */
int anys(s1, s2)
register char *s1, *s2;
{
    while (*s1)
	if (any(*s1++, s2))
	    return (1);
    return (0);
}

/*
 * Is character c in s?
 */
int any(c, s)
register int c;
register char *s;
{
    while (*s)
	if (*s++ == c)
	    return (1);
    return (0);
}

char *putn(n)
register int n;
{
    return (itoa(n, -1));
}

char *itoa(u, n)
register unsigned u;
int n;
{
    register char *cp;
    static char s[20];
    int m;

    m = 0;
    if (n < 0 && (int) u < 0) {
	m++;
	u = -u;
    }
    cp = s + sizeof(s);
    *--cp = 0;
    do {
	*--cp = u % 10 + '0';
	u /= 10;
    } while (--n > 0 || u);
    if (m)
	*--cp = '-';

    return (cp);
}

int letter(c)
register c;
{
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_');
}

int digit(c)
register c;
{
    return (c >= '0' && c <= '9');
}

int letnum(c)
register c;
{
    return (letter(c) || digit(c));
}

char *space(n)
int n;
{
    register char *cp;

    if ((cp = getcell(n)) == 0)
	err("out of string space");

    return (cp);
}

char *strerror(int num)
{
    extern int sys_ner;
    extern char *sys_err[];
    
    if (num >= 0 && num < sys_ner)
	return sys_err[num];
    else
	return "Unknown error number";
}

void abort()
{
    exit(-1);
}

int isatty(int fd)
{
    return 1;
}
