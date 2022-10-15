#include <unix.h>

/* -------- gmatch.c -------- */

static char *cclass(char *p, int sub);

/*
 * int gmatch(string, pattern)
 * char *string, *pattern;
 *
 * Match a pattern as in sh(1).
 */

#define	CMASK	0377
#define	QUOTE	0200
#define	QMASK	(CMASK&~QUOTE)
#define	NOT	'!'		/* might use ^ */

int gmatch(s, p)
register char *s, *p;
{
    register int sc, pc;

    if (s == NULL || p == NULL) return (0);

    while ((pc = *p++ & CMASK) != '\0') {
	sc = *s++ & QMASK;
	switch (pc) {
	case '[':
	    if ((p = cclass(p, sc)) == NULL) return (0);
	    break;

	case '?':
	    if (sc == 0) return (0);
	    break;

	case '*':
	    s--;
	    do {
		if (*p == '\0' || gmatch(s, p)) return (1);
	    } while (*s++ != '\0');
	    return (0);

	default:
	    if (sc != (pc & ~QUOTE)) return (0);
	}
    }
    return (*s == 0);
}

static char *cclass(p, sub)
register char *p;
register int sub;
{
    register int c, d, not, found;

    if ((not = *p == NOT) != 0) p++;
    found = not;
    do {
	if (*p == '\0')
	    return ((char *) NULL);
	c = *p & CMASK;
	if (p[1] == '-' && p[2] != ']') {
	    d = p[2] & CMASK;
	    p++;
	} else {
	    d = c;
	}
	if (c == sub || (c <= sub && sub <= d)) found = !not;
    } while (*++p != ']');

    return (found ? p + 1 : (char *) NULL);
}
