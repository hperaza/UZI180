#include <setjmp.h>
#include <unix.h>

/* -------- var.c -------- */
#include "sh.h"

static char *findeq(char *cp);

/*
 * Find the given name in the dictionary
 * and return its value.  If the name was
 * not previously there, enter it now and
 * return a null value.
 */
struct var *lookup(n)
register char *n;
{
    register struct var *vp;
    register char *cp;
    register int c;
    static struct var dummy;

    if (digit(*n)) {
	dummy.name = n;
	for (c = 0; digit(*n) && c < 1000; n++)
	    c = c * 10 + *n - '0';
	dummy.status = RONLY;
	dummy.value = c <= dolc ? dolv[c] : null;
	return (&dummy);
    }
    for (vp = vlist; vp; vp = vp->next)
	if (eqname(vp->name, n)) return (vp);
    cp = findeq(n);
    vp = (struct var *) space(sizeof(*vp));
    if (vp == 0 || (vp->name = space((int) (cp - n) + 2)) == 0) {
	dummy.name = dummy.value = "";
	return (&dummy);
    }
    for (cp = vp->name; (*cp = *n++) && *cp != '='; cp++);
    if (*cp == 0) *cp = '=';
    *++cp = 0;
    setarea((char *) vp, 0);
    setarea((char *) vp->name, 0);
    vp->value = null;
    vp->next = vlist;
    vp->status = GETCELL;
    vlist = vp;

    return (vp);
}

/*
 * give variable at `vp' the value `val'.
 */
void setval(vp, val)
struct var *vp;
char *val;
{
    nameval(vp, val, (char *) NULL);
}

/*
 * if name is not NULL, it must be a prefix of the space `val',
 * and end with `='.
 * this is all so that exporting values is reasonably painless.
 */
void nameval(vp, val, name)
register struct var *vp;
char *val, *name;
{
    register char *cp, *xp;
    char *nv;
    int fl;

    if (vp->status & RONLY) {
	for (xp = vp->name; *xp && *xp != '=';)
	    putc(*xp++);
	err(" is read-only");
	return;
    }
    fl = 0;
    if (name == NULL) {
	xp = space(strlen(vp->name) + strlen(val) + 2);
	if (xp == 0) return;
	/* make string:  name=value */
	setarea((char *) xp, 0);
	name = xp;
	for (cp = vp->name; (*xp = *cp++) && *xp != '='; xp++);
	if (*xp++ == 0) xp[-1] = '=';
	nv = xp;
	for (cp = val; (*xp++ = *cp++) != '\0';);
	val = nv;
	fl = GETCELL;
    }
    if (vp->status & GETCELL)
	xfree(vp->name);	/* form new string `name=value' */
    vp->name = name;
    vp->value = val;
    vp->status |= fl;
}

void export(vp)
struct var *vp;
{
    vp->status |= EXPORT;
}

void ronly(vp)
struct var *vp;
{
    if (letter(vp->name[0]))	/* not an internal symbol ($# etc) */
	vp->status |= RONLY;
}

int isassign(s)
register char *s;
{
    if (!letter((int) *s))
	return (0);
    for (; *s != '='; s++)
	if (*s == 0 || !letnum(*s))
	    return (0);
    return (1);
}

int assign(s, cf)
register char *s;
int cf;
{
    register char *cp;
    struct var *vp;

    if (!letter(*s)) return (0);
    for (cp = s; *cp != '='; cp++)
	if (*cp == 0 || !letnum(*cp)) return (0);
    vp = lookup(s);
    nameval(vp, ++cp, cf == COPYV ? (char *) NULL : s);
    if (cf != COPYV)
	vp->status &= ~GETCELL;
    return (1);
}

int checkname(cp)
register char *cp;
{
    if (!letter(*cp++)) return (0);
    while (*cp)
	if (!letnum(*cp++)) return (0);
    return (1);
}

void putvlist(f, out)
register int f, out;
{
    register struct var *vp;

    for (vp = vlist; vp; vp = vp->next)
	if (vp->status & f && letter(*vp->name)) {
	    if (vp->status & EXPORT)
		write(out, "export ", 7);
	    if (vp->status & RONLY)
		write(out, "readonly ", 9);
	    write(out, vp->name, (int) (findeq(vp->name) - vp->name));
	    write(out, "\n", 1);
	}
}

int eqname(n1, n2)
register char *n1, *n2;
{
    for (; *n1 != '=' && *n1 != 0; n1++)
	if (*n2++ != *n1)
	    return (0);
    return (*n2 == 0 || *n2 == '=');
}

static char *findeq(cp)
register char *cp;
{
    while (*cp != '\0' && *cp != '=')
	cp++;
    return (cp);
}
