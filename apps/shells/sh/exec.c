#include <unix.h>
#include <setjmp.h>

/*#undef NULL*/

extern int fork();

extern int doexec(struct op *t);


/* -------- exec.c -------- */
#include "sh.h"

int isbreak;

/*
 * execute tree
 */

static int forkexec(struct op *t, int *pin, int *pout, int act, char **wp,
		    int *pforked);
static int parent(void);
extern int iosetup(struct ioword *iop, int pipein, int pipeout);
static void echo(char **wp);
static struct op **find1case(struct op *t, char *w);
static struct op *findcase(struct op *t, char *w);
static void brkset(struct brkcon *bc);

static void _tasync(int *pin, int *pout, int *rv, struct op *t);
static void _tif(int *pin, int *pout, int *rv, struct op *t);
static int  _tfor(int *pin, int *pout, int *rv, struct op *t, char **wp);
static int  _twhile(int *pin, int *pout, int *rv, struct op *t);

int execute(t, pin, pout, act)
register struct op *t;
int *pin, *pout;
int act;
{
    register struct op *t1;
    int i, pv[2], rv, child, a;
    char *cp, **wp, **wp2;

    if (t == NULL)
	return (0);
    rv = 0;
    a = areanum++;
    wp = (wp2 = t->words) != NULL
	? eval(wp2, t->type == TCOM ? DOALL : DOALL & ~DOKEY)
	: NULL;

    switch (t->type) {
    case TPAREN:
    case TCOM:
	rv = forkexec(t, pin, pout, act, wp, &child);
	if (child) {
	    exstat = rv;
	    leave();
	}
	break;

    case TPIPE:
	if ((rv = openpipe(pv)) < 0)
	    break;
	pv[0] = remap(pv[0]);
	pv[1] = remap(pv[1]);
	(void) execute(t->left, pin, pv, 0);
	rv = execute(t->right, pv, pout, 0);
	break;

    case TLIST:
	(void) execute(t->left, pin, pout, 0);
	rv = execute(t->right, pin, pout, 0);
	break;

    case TASYNC:
	_tasync(pin, pout, &rv, t);
	break;

    case TOR:
    case TAND:
	rv = execute(t->left, pin, pout, 0);
	if ((t1 = t->right) != NULL && (rv == 0) == (t->type == TAND))
	    rv = execute(t1, pin, pout, 0);
	break;

    case TFOR:
	if (_tfor(pin, pout, &rv, t, wp)) goto broken;
	break;

    case TWHILE:
    case TUNTIL:
	if (_twhile(pin, pout, &rv, t)) goto broken;
	break;

    case TIF:
    case TELIF:
	_tif(pin, pout, &rv, t);
	break;

    case TCASE:
	if ((cp = evalstr(t->str, DOSUB | DOTRIM)) == 0)
	    cp = "";
	if ((t1 = findcase(t->left, cp)) != NULL)
	    rv = execute(t1, pin, pout, 0);
	break;

    case TBRACE:
/*
	if (iopp = t->ioact)
	    while (*iopp)
		if (iosetup(*iopp++, pin!=NULL, pout!=NULL)) {
		    rv = -1;
		    break;
		}
*/
	if (rv >= 0 && (t1 = t->left))
	    rv = execute(t1, pin, pout, 0);
	break;
    }

  broken:
    t->words = wp2;
    isbreak = 0;
    freehere(areanum);
    freearea(areanum);
    areanum = a;
    if (talking && intr) {
	closeall();
	fail();
    }
    if ((i = trapset) != 0) {
	trapset = 0;
	runtrap(i);
    }
    return (rv);
}

static void _tasync(int *pin, int *pout, int *rv, struct op *t)
{
    int i;

    i = parent();
    if (i != 0) {
	if (i != -1) {
	    setval(lookup("!"), putn(i));
	    if (pin != NULL)
		closepipe(pin);
	    if (talking) {
		prs(putn(i));
		prs("\n");
	    }
	} else {
	    *rv = -1;
	}
	setstatus(*rv);
    } else {
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	if (talking)
	    signal(SIGTERM, SIG_DFL);
	talking = 0;
	if (pin == NULL) {
	    close(0);
	    open("/dev/null", 0);
	}
	exit(execute(t->left, pin, pout, FEXEC));
    }
}

static void _tif(int *pin, int *pout, int *rv, struct op *t)
{
    if (t->right != NULL) {
	*rv = !execute(t->left, pin, pout, 0) ?
	       execute(t->right->left, pin, pout, 0) :
	       execute(t->right->right, pin, pout, 0);
    }
}

static int _twhile(int *pin, int *pout, int *rv, struct op *t)
{
    register struct op *t1;
    struct brkcon bc;
    
    while (setjmp(bc.brkpt))
	if (isbreak)
	    return 1;
    brkset(&bc);
    t1 = t->left;
    while ((execute(t1, pin, pout, 0) == 0) == (t->type == TWHILE))
	*rv = execute(t->right, pin, pout, 0);
    brklist = brklist->nextlev;
    return 0;
}

static int _tfor(int *pin, int *pout, int *rv, struct op *t, char **wp)
{
    register struct op *t1;
    struct var *vp;
    int i;
    struct brkcon bc;

    if (wp == NULL) {
	wp = dolv + 1;
	if ((i = dolc) < 0)
	    i = 0;
    } else {
	i = -1;
	while (*wp++ != NULL);
    }
    vp = lookup(t->str);
    while (setjmp(bc.brkpt))
	if (isbreak)
	    return 1;
    brkset(&bc);
    for (t1 = t->left; i-- && *wp != NULL;) {
	setval(vp, *wp++);
	*rv = execute(t1, pin, pout, 0);
    }
    brklist = brklist->nextlev;
    return 0;
}

static int forkexec(t, pin, pout, act, wp, pforked)
register struct op *t;
int *pin, *pout;
int act;
char **wp;
int *pforked;
{
    int i, rv, (*shcom) ();
    register int f;
    char *cp;
    struct ioword **iopp;
    int resetsig;
    char **owp;

    owp = wp;
    resetsig = 0;
    *pforked = 0;
    shcom = NULL;
    rv = -1;			/* system-detected error */
    if (t->type == TCOM) {
	while ((cp = *wp++) != NULL);
	cp = *wp;

	/* strip all initial assignments */
	/* not correct wrt PATH=yyy command  etc */
	if (flag['x'])
	    echo(cp ? wp : owp);
	if (cp == NULL && t->ioact == NULL) {
	    while ((cp = *owp++) != NULL && assign(cp, COPYV));
	    return (setstatus(0));
	} else if (cp != NULL)
	    shcom = inbuilt(cp);
    }
    t->words = wp;
    f = act;
    if (shcom == NULL && (f & FEXEC) == 0) {
	i = parent();
	if (i != 0) {
	    if (i == -1)
		return (rv);
	    if (pin != NULL)
		closepipe(pin);
	    return (pout == NULL ? setstatus(waitfor(i, 0)) : 0);
	}
	if (talking) {
	    signal(SIGINT, SIG_IGN);
	    signal(SIGQUIT, SIG_IGN);
	    resetsig = 1;
	}
	talking = 0;
	intr = 0;
	(*pforked)++;
	brklist = 0;
	execflg = 0;
    }
    if (owp != NULL)
	while ((cp = *owp++) != NULL && assign(cp, COPYV))
	    if (shcom == NULL)
		export(lookup(cp));
#ifdef COMPIPE
    if ((pin != NULL || pout != NULL) && shcom != NULL && shcom != doexec) {
	err("piping to/from shell builtins not yet done");
	return (-1);
    }
#endif
    if (pin != NULL) {
	dup2(pin[0], 0);
	closepipe(pin);
    }
    if (pout != NULL) {
	dup2(pout[1], 1);
	closepipe(pout);
    }
    if ((iopp = t->ioact) != NULL) {
	if (shcom != NULL && shcom != doexec) {
	    prs(cp);
	    err(": cannot redirect shell command");
	    return (-1);
	}
	while (*iopp)
	    if (iosetup(*iopp++, pin != NULL, pout != NULL))
		return (rv);
    }
    if (shcom)
	return (setstatus((*shcom) (t)));
    /* should use FIOCEXCL */
    for (i = FDBASE; i < NOFILE; i++)
	close(i);
    if (resetsig) {
	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
    }
    if (t->type == TPAREN)
	exit(execute(t->left, NOPIPE, NOPIPE, FEXEC));
    if (wp[0] == NULL)
	exit(0);
    cp = rexecve(wp[0], wp, makenv());
    prs(wp[0]);
    prs(": ");
    warn(cp);
    if (!execflg)
	trap[0] = NULL;
    leave();
    /* NOTREACHED */
}

/*
 * common actions when creating a new child
 */
static int parent()
{
    register int i;

    i = fork();
    if (i != 0) {
	if (i == -1)
	    warn("try again");
    }
    return (i);
}

static void echo(wp)
register char **wp;
{
    register i;

    prs("+");
    for (i = 0; wp[i]; i++) {
	if (i)
	    prs(" ");
	prs(wp[i]);
    }
    prs("\n");
}

static struct op **find1case(t, w)
struct op *t;
char *w;
{
    register struct op *t1;
    struct op **tp;
    register char **wp, *cp;

    if (t == NULL)
	return ((struct op **) NULL);
    if (t->type == TLIST) {
	if ((tp = find1case(t->left, w)) != NULL)
	    return (tp);
	t1 = t->right;		/* TPAT */
    } else
	t1 = t;
    for (wp = t1->words; *wp;)
	if ((cp = evalstr(*wp++, DOSUB)) && gmatch(w, cp))
	    return (&t1->left);
    return ((struct op **) NULL);
}

static struct op *findcase(t, w)
struct op *t;
char *w;
{
    register struct op **tp;

    return ((tp = find1case(t, w)) != NULL ? *tp : (struct op *) NULL);
}

/*
 * Enter a new loop level (marked for break/continue).
 */
static void brkset(bc)
struct brkcon *bc;
{
    bc->nextlev = brklist;
    brklist = bc;
}
