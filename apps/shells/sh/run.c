#include <setjmp.h>
#include <unix.h>

extern int open(), creat(), dup2(), wait();

/*---------------------------------*/
#include "sh.h"

static char *signame[] = {
    "Signal 0",
    "Hangup",
    (char *) NULL,		/* interrupt */
    "Quit",
    "Illegal instruction",
    "Trace/BPT trap",
    "Abort",
    "EMT trap",
    "Floating exception",
    "Killed",
    "Bus error",
    "Memory fault",
    "Bad system call",
    (char *) NULL,		/* broken pipe */
    "Alarm clock",
    "Terminated",
};
#define	NSIGNAL (sizeof(signame)/sizeof(signame[0]))


/*
 * 0< 1> are ignored as required
 * within pipelines.
 */
int iosetup(iop, pipein, pipeout)
register struct ioword *iop;
int pipein, pipeout;
{
    register u;
    char *cp, *msg;

    if (iop->io_unit == IODEFAULT)	/* take default */
	iop->io_unit = iop->io_flag & (IOREAD | IOHERE) ? 0 : 1;
    if (pipein && iop->io_unit == 0)
	return (0);
    if (pipeout && iop->io_unit == 1)
	return (0);
    msg = iop->io_flag & (IOREAD | IOHERE) ? "open" : "create";
    if ((iop->io_flag & IOHERE) == 0) {
	cp = iop->io_name;
	if ((cp = evalstr(cp, DOSUB | DOTRIM)) == NULL)
	    return (1);
    }
    if (iop->io_flag & IODUP) {
	if (cp[1] || (!digit(*cp) && *cp != '-')) {
	    prs(cp);
	    err(": illegal >& argument");
	    return (1);
	}
	if (*cp == '-')
	    iop->io_flag = IOCLOSE;
	iop->io_flag &= ~(IOREAD | IOWRITE);
    }
    switch (iop->io_flag) {
    case IOREAD:
	u = open(cp, 0);
	break;

    case IOHERE:
    case IOHERE | IOXHERE:
	u = herein(iop->io_name, iop->io_flag & IOXHERE);
	cp = "here file ";
	break;

    case IOWRITE | IOCAT:
	if ((u = open(cp, 1)) >= 0) {
	    lseek(u, (long) 0, 2);
	    break;
	}
    case IOWRITE:
	u = creat(cp, 0666);
	break;

    case IODUP:
	u = dup2(*cp - '0', iop->io_unit);
	break;

    case IOCLOSE:
	close(iop->io_unit);
	return (0);
    }
    if (u < 0) {
	int e = errno;
	prs(cp);
	if (iop->io_flag & IOHERE)
	    prs(iop->io_name);
	prs(": cannot ");
	prs(msg);
	prs(" (");
	prs(strerror(e));
	warn(")");
	return (1);
    } else {
	if (u != iop->io_unit) {
	    dup2(u, iop->io_unit);
	    close(u);
	}
    }
    return (0);
}

/*
 * Wait for the last process created.
 * Print a message for each process found
 * that was killed by a signal.
 * Ignore interrupt signals while waiting
 * unless `canintr' is true.
 */
int waitfor(lastpid, canintr)
register int lastpid;
int canintr;
{
    register int pid, rv;
    int s;
    int oheedint = heedint;

    heedint = 0;
    rv = 0;
    do {
	pid = wait(&s);
	if (pid == -1) {
	    if (errno != EINTR || canintr)
		break;
	} else {
	    if ((rv = WAITSIG(s)) != 0) {
		if (rv < NSIGNAL) {
		    if (signame[rv] != NULL) {
			if (pid != lastpid) {
			    prn(pid);
			    prs(": ");
			}
			prs(signame[rv]);
		    }
		} else {
		    if (pid != lastpid) {
			prn(pid);
			prs(": ");
		    }
		    prs("Signal ");
		    prn(rv);
		    prs(" ");
		}
		if (WAITCORE(s))
		    prs(" - core dumped");
		if (rv >= NSIGNAL || signame[rv])
		    prs("\n");
		rv = -1;
	    } else
		rv = WAITVAL(s);
	}
    } while (pid != lastpid);
    heedint = oheedint;
    if (intr)
	if (talking) {
	    if (canintr)
		intr = 0;
	} else {
	    if (exstat == 0)
		exstat = rv;
	    onintr(0);
	}
    return (rv);
}

int setstatus(s)
register int s;
{
    exstat = s;
    setval(lookup("?"), putn(s));
    return (s);
}

/*
 * PATH-searching interface to execve.
 * If getenv("PATH") were kept up-to-date,
 * execvp might be used.
 */
char *rexecve(c, v, envp)
char *c, **v, **envp;
{
    register int i;
    register char *sp, *tp;
    int eacces = 0, asis = 0;

    sp = any('/', c) ? "" : path->value;
    asis = *sp == '\0';
    while (asis || *sp != '\0') {
	asis = 0;
	tp = e.linep;
	for (; *sp != '\0'; tp++)
	    if ((*tp = *sp++) == ':') {
		asis = *sp == '\0';
		break;
	    }
	if (tp != e.linep)
	    *tp++ = '/';
	for (i = 0; (*tp++ = c[i++]) != '\0';);
	execve(e.linep, v, envp);
	switch (errno) {
	case ENOEXEC:
	    *v = e.linep;
	    tp = *--v;
	    *v = e.linep;
	    execve("/bin/sh", v, envp);
	    *v = tp;
	    return ("no Shell");

	case ENOMEM:
	    return ("program too big");

	case E2BIG:
	    return ("argument list too long");

	case EACCES:
	    eacces++;
	    break;
	}
    }
    return (errno == ENOENT ? "not found" : "cannot execute");
}

/*
 * Run the command produced by generator `f'
 * applied to stream `arg'.
 */
int run(argp, f)
struct ioarg *argp;
int (*f) ();
{
    struct op *otree;
    struct wdblock *swdlist;
    struct wdblock *siolist;
    jmp_buf ev, rt;
    xint *ofail;
    int rv;

    areanum++;
    swdlist = wdlist;
    siolist = iolist;
    otree = outtree;
    ofail = failpt;
    rv = -1;
    if (newenv(setjmp(errpt = ev)) == 0) {
	wdlist = 0;
	iolist = 0;
	pushio(argp, f);
	e.iobase = e.iop;
	yynerrs = 0;
	if (setjmp(failpt = rt) == 0 && yyparse() == 0)
	    rv = execute(outtree, NOPIPE, NOPIPE, 0);
	quitenv();
    }
    wdlist = swdlist;
    iolist = siolist;
    failpt = ofail;
    outtree = otree;
    freearea(areanum--);

    return (rv);
}
