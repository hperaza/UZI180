#include <setjmp.h>
#include <unix.h>

int creat(), open(), getpid();

/* -------- here.c -------- */
#include "sh.h"

static void readhere(char **name, char *s, int ec);
void tempname(char *tname);

/*
 * here documents
 */

struct here {
    char *h_tag;
    int h_dosub;
    struct ioword *h_iop;
    struct here *h_next;
};

static struct here *inhere;	/* list of hear docs while parsing */
static struct here *acthere;	/* list of active here documents */

void inithere()
{
    inhere = acthere = (struct here *) 0;
}

void markhere(s, iop)
register char *s;
struct ioword *iop;
{
    register struct here *h, *lh;

    h = (struct here *) space(sizeof(struct here));
    if (h == 0) return;
    h->h_tag = evalstr(s, DOSUB);
    if (h->h_tag == 0) return;
    h->h_iop = iop;
    iop->io_name = 0;
    h->h_next = NULL;
    if (inhere == 0) {
	inhere = h;
    } else {
	for (lh = inhere; lh != NULL; lh = lh->h_next)
	    if (lh->h_next == 0) {
		lh->h_next = h;
		break;
	    }
    }
    iop->io_flag |= IOHERE | IOXHERE;
    for (s = h->h_tag; *s; s++)
	if (*s & QUOTE) {
	    iop->io_flag &= ~IOXHERE;
	    *s &= ~QUOTE;
	}
    h->h_dosub = iop->io_flag & IOXHERE;
}

void gethere()
{
    register struct here *h, *hp;

    /* Scan here files first leaving inhere list in place */
    for (hp = h = inhere; h != NULL; hp = h, h = h->h_next)
	readhere(&h->h_iop->io_name, h->h_tag, h->h_dosub ? 0 : '\'');

    /* Make inhere list active - keep list intact for scraphere */
    if (hp != NULL) {
	hp->h_next = acthere;
	acthere = inhere;
	inhere = NULL;
    }
}

static void readhere(name, s, ec)
char **name;
register char *s;
int ec;
{
    int tf;
    char tname[30];
    register c;
    jmp_buf ev;
    char *next, line[LINELIM + 1];

    tempname(tname);
    *name = strsave(tname, areanum);
    tf = creat(tname, 0600);
    if (tf < 0)
	return;
    if (newenv(setjmp(errpt = ev)) != 0) {
	unlink(tname);
    } else {
	pushio(e.iop->argp, e.iop->iofn);
	e.iobase = e.iop;
	for (;;) {
	    if (talking && e.iop <= iostack)
		prs(cprompt->value);
	    next = line;
	    while ((c = readc()) != '\n' && c) {
		if (next >= &line[LINELIM]) {
		    c = 0;
		    break;
		}
		*next++ = c;
	    }
	    *next = 0;
	    if (strcmp(s, line) == 0 || c == 0)
		break;
	    *next++ = '\n';
	    write(tf, line, (int) (next - line));
	}
	if (c == 0) {
	    prs("here document `");
	    prs(s);
	    err("' unclosed");
	}
	quitenv();
    }
    close(tf);
}

/*
 * open here temp file.
 * if unquoted here, expand here temp file into second temp file.
 */
int herein(hname, xdoll)
char *hname;
int xdoll;
{
    register hf, tf;

    if (hname == 0)
	return (-1);
    hf = open(hname, 0);
    if (hf < 0)
	return (-1);
    if (xdoll) {
	char c;
	char tname[30];
	jmp_buf ev;

	tempname(tname);
	if ((tf = creat(tname, 0600)) < 0)
	    return (-1);
	if (newenv(setjmp(errpt = ev)) == 0) {
	    PUSHIO(afile, hf, herechar);
	    setbase(e.iop);
	    while ((c = subgetc(0, 0)) != 0) {
		char c1 = c & ~QUOTE;

		if (c & QUOTE && !any(c1, "`$\\"))
		    write(tf, "\\", 1);
		write(tf, &c1, 1);
	    }
	    quitenv();
	} else {
	    unlink(tname);
	}
	close(tf);
	tf = open(tname, 0);
	unlink(tname);
	return (tf);
    } else {
	return (hf);
    }
}

void scraphere()
{
    register struct here *h;

    for (h = inhere; h != NULL; h = h->h_next) {
	if (h->h_iop && h->h_iop->io_name)
	    unlink(h->h_iop->io_name);
    }
    inhere = NULL;
}

/* unlink here temp files before a freearea(area) */
void freehere(area)
int area;
{
    register struct here *h, *hl;

    hl = NULL;
    for (h = acthere; h != NULL; h = h->h_next) {
	if (getarea((char *) h) >= area) {
	    if (h->h_iop->io_name != NULL)
		unlink(h->h_iop->io_name);
	    if (hl == NULL)
		acthere = h->h_next;
	    else
		hl->h_next = h->h_next;
	} else {
	    hl = h;
	}
    }
}

void tempname(tname)
char *tname;
{
    static int inc;
    register char *cp, *lp;

    for (cp = tname, lp = "/tmp/shtm"; (*cp = *lp++) != '\0'; cp++);
    lp = putn(getpid() * 1000 + inc++);
    for (; (*cp = *lp++) != '\0'; cp++);
}
