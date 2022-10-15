#include <setjmp.h>
#include <unix.h>

int chdir(), umask(), open(), read();
extern int (* signal(int, int (*)()))();

/* -------- do.c -------- */
#include "sh.h"

char ourtrap[NSIGS+1];

int  dolabel(void);
int  dochdir(struct op *t);
int  doshift(struct op *t);
int  dologin(struct op *t);
int  doumask(struct op *t);
int  doexec(struct op *t);
int  dodot(struct op *t);
int  dowait(struct op *t);
int  doread(struct op *t);
int  doeval(struct op *t);
int  dotrap(struct op *t);
int  getn(char *as);
int  getsig(char *s);
/*void setsig(int n, void (*f)(int));*/
int  dobreak(struct op *t);
int  docontinue(struct op *t);
static int brkcontin(char *cp, int val);
int  doexit(struct op *t);
int  doexport(struct op *t);
int  doreadonly(struct op *t);
static void rdexp(char **wp, void (*f)(), int key);
static void badid(char *s);
int  doset(struct op *t);
void varput(char *s, int out);
int  dotimes(void);

void setsig(n, f)
register int n;
void (*f)(int);
{
    if (n == 0) return;
    if (signal(n, SIG_IGN) != SIG_IGN || ourtrap[n]) {
	ourtrap[n] = 1;
	signal(n, f);
    }
}



/*
 * built-in commands: doX
 */

int dolabel()
{
    return (0);
}

int dochdir(t)
register struct op *t;
{
    register char *cp, *er;

    if ((cp = t->words[1]) == NULL && (cp = homedir->value) == NULL)
	er = ": no home directory";
    else if (chdir(cp) < 0)
	er = ": bad directory";
    else
	return (0);

    prs(cp != NULL ? cp : "cd");
    err(er);

    return (1);
}

int doshift(t)
register struct op *t;
{
    register n;

    n = t->words[1] ? getn(t->words[1]) : 1;
    if (dolc < n) {
	err("nothing to shift");
	return (1);
    }
    dolv[n] = dolv[0];
    dolv += n;
    dolc -= n;
    setval(lookup("#"), putn(dolc));

    return (0);
}

/*
 * execute login and newgrp directly
 */
int dologin(t)
struct op *t;
{
    register char *cp;

    if (talking) {
	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
    }
    cp = rexecve(t->words[0], t->words, makenv());
    prs(t->words[0]);
    prs(": ");
    err(cp);

    return (1);
}

int doumask(t)
register struct op *t;
{
    register int i, n;
    register char *cp;

    if ((cp = t->words[1]) == NULL) {
	i = umask(0);
	umask(i);
	for (n = 3 * 4; (n -= 3) >= 0;)
	    putc('0' + ((i >> n) & 07));
	putc('\n');
    } else {
	for (n = 0; *cp >= '0' && *cp <= '9'; cp++)
	    n = n * 8 + (*cp - '0');
	umask(n);
    }
    return (0);
}

int doexec(t)
register struct op *t;
{
    register i;
    jmp_buf ex;
    xint *ofail;

    t->ioact = NULL;

    for (i = 0; (t->words[i] = t->words[i + 1]) != NULL; i++);
    if (i == 0) return (1);

    execflg = 1;
    ofail = failpt;

    if (setjmp(failpt = ex) == 0)
	execute(t, NOPIPE, NOPIPE, FEXEC);

    failpt = ofail;
    execflg = 0;

    return (1);
}

int dodot(t)
struct op *t;
{
    register i;
    register char *sp, *tp;
    char *cp;

    if ((cp = t->words[1]) == NULL) return (0);

    sp = any('/', cp) ? ":" : path->value;
    while (*sp) {
	tp = e.linep;
	while (*sp && (*tp = *sp++) != ':')
	    tp++;
	if (tp != e.linep)
	    *tp++ = '/';
	for (i = 0; (*tp++ = cp[i++]) != '\0';);
	if ((i = open(e.linep, 0)) >= 0) {
	    exstat = 0;
	    next(remap(i));
	    return (exstat);
	}
    }
    prs(cp);
    err(": not found");

    return (-1);
}

int dowait(t)
struct op *t;
{
    register i;
    register char *cp;

    if ((cp = t->words[1]) != NULL) {
	i = getn(cp);
	if (i == 0) return (0);
    } else {
	i = -1;
    }
    setstatus(waitfor(i, 1));

    return (0);
}

int doread(t)
struct op *t;
{
    register char *cp, **wp;
    register nb;
    register int nl = 0;

    if (t->words[1] == NULL) {
	err("Usage: read name ...");
	return (1);
    }
    for (wp = t->words + 1; *wp; wp++) {
	for (cp = e.linep; !nl && cp < elinep - 1; cp++)
	    if ((nb = read(0, cp, sizeof(*cp))) != sizeof(*cp) ||
		(nl = (*cp == '\n')) || (wp[1] && any(*cp, ifs->value)))
		break;
	*cp = 0;
	if (nb <= 0)
	    break;
	setval(lookup(*wp), e.linep);
    }
    return (nb <= 0);
}

int doeval(t)
register struct op *t;
{
    return (RUN(awordlist, t->words + 1, wdchar));
}

int dotrap(t)
register struct op *t;
{
    register int n, i;
    register int resetsig;

    if (t->words[1] == NULL) {
	for (i = 0; i <= NSIGS; i++)
	    if (trap[i]) {
		prn(i);
		prs(": ");
		prs(trap[i]);
		prs("\n");
	    }
	return (0);
    }

    resetsig = digit(*t->words[1]);

    for (i = resetsig ? 1 : 2; t->words[i] != NULL; ++i) {
	n = getsig(t->words[i]);
	xfree(trap[n]);
	trap[n] = 0;
	if (!resetsig) {
	    if (*t->words[1] != '\0') {
		trap[n] = strsave(t->words[1], 0);
		setsig(n, sig);
	    } else {
		setsig(n, SIG_IGN);
	    }
	} else {
	    if (talking) {
		if (n == SIGINT)
		    setsig(n, onintr);
		else
		    setsig(n, n == SIGQUIT ? SIG_IGN : SIG_DFL);
	    } else {
		setsig(n, SIG_DFL);
	    }
	}
    }
    return (0);
}

int getsig(s)
char *s;
{
    register int n;

    if ((n = getn(s)) < 0 || n > NSIGS) {
	err("trap: bad signal number");
	n = 0;
    }
    return (n);
}

int getn(as)
char *as;
{
    register char *s;
    register n, m;

    s = as;
    m = 1;
    if (*s == '-') {
	m = -1;
	s++;
    }
    for (n = 0; digit(*s); s++)
	n = (n * 10) + (*s - '0');
    if (*s) {
	prs(as);
	err(": bad number");
    }
    return (n * m);
}

int dobreak(t)
struct op *t;
{
    return (brkcontin(t->words[1], 1));
}

int docontinue(t)
struct op *t;
{
    return (brkcontin(t->words[1], 0));
}

static int brkcontin(cp, val)
register char *cp;
int val;
{
    register struct brkcon *bc;
    register nl;

    nl = cp == NULL ? 1 : getn(cp);
    if (nl <= 0)
	nl = 999;
    do {
	if ((bc = brklist) == NULL)
	    break;
	brklist = bc->nextlev;
    } while (--nl);
    if (nl) {
	err("bad break/continue level");
	return (1);
    }
    isbreak = val;
    longjmp(bc->brkpt, 1);
    /* NOTREACHED */
}

int doexit(t)
struct op *t;
{
    register char *cp;

    execflg = 0;
    if ((cp = t->words[1]) != NULL)
	setstatus(getn(cp));
    leave();
    /* NOTREACHED */
}

int doexport(t)
struct op *t;
{
    rdexp(t->words + 1, export, EXPORT);
    return (0);
}

int doreadonly(t)
struct op *t;
{
    rdexp(t->words + 1, ronly, RONLY);
    return (0);
}

static void rdexp(wp, f, key)
register char **wp;
void (*f) ();
int key;
{
    if (*wp != NULL) {
	for (; *wp != NULL; wp++)
	    if (checkname(*wp))
		(*f) (lookup(*wp));
	    else
		badid(*wp);
    } else {
	putvlist(key, 1);
    }
}

static void badid(s)
register char *s;
{
    prs(s);
    err(": bad identifier");
}

int doset(t)
register struct op *t;
{
    register struct var *vp;
    register char *cp;
    register n;

    if ((cp = t->words[1]) == NULL) {
	for (vp = vlist; vp; vp = vp->next)
	    varput(vp->name, 1);
	return (0);
    }
    if (*cp == '-') {
	/* bad: t->words++; */
	for (n = 0; (t->words[n] = t->words[n + 1]) != NULL; n++);
	if (*++cp == 0) {
	    flag['x'] = flag['v'] = 0;
	} else {
	    for (; *cp; cp++)
		switch (*cp) {
		case 'e':
		    if (!talking) flag['e']++;
		    break;

		default:
		    if (*cp >= 'a' && *cp <= 'z') flag[*cp]++;
		    break;
		}
	}
	setdash();
    }
    if (t->words[1]) {
	t->words[0] = dolv[0];
	for (n = 1; t->words[n]; n++)
	    setarea((char *) t->words[n], 0);
	dolc = n - 1;
	dolv = t->words;
	setval(lookup("#"), putn(dolc));
	setarea((char *) (dolv - 1), 0);
    }
    return (0);
}

void varput(s, out)
register char *s;
int out;
{
    if (letnum(*s)) {
	write(out, s, strlen(s));
	write(out, "\n", 1);
    }
}


#define	SECS	60L
#define	MINS	3600L

int dotimes()
{
#if 0   /* rewrite for UZI! */
    struct tms tbuf;

    times(&tbuf);

    prn((int) (tbuf.tms_cutime / MINS));
    prs("m");
    prn((int) ((tbuf.tms_cutime % MINS) / SECS));
    prs("s ");
    prn((int) (tbuf.tms_cstime / MINS));
    prs("m");
    prn((int) ((tbuf.tms_cstime % MINS) / SECS));
    prs("s\n");
    return (0);
#endif
}

struct builtin {
    char *command;
    int (*fn) ();
};

static struct builtin builtin[] = {
    ":",        dolabel,
    "cd",       dochdir,
    "shift",    doshift,
    "exec",     doexec,
    "wait",     dowait,
    "read",     doread,
    "eval",     doeval,
    "trap",     dotrap,
    "break",    dobreak,
    "continue", docontinue,
    "exit",     doexit,
    "export",   doexport,
    "readonly", doreadonly,
    "set",      doset,
    ".",        dodot,
    "umask",    doumask,
    "login",    dologin,
    "newgrp",   dologin,
    "times",    dotimes,
    0,          0
};

int (*inbuilt(s)) ()
register char *s;
{
    register struct builtin *bp;

    for (bp = builtin; bp->command != NULL; bp++)
	if (strcmp(bp->command, s) == 0)
	    return (bp->fn);
    return ((int (*)()) NULL);
}
