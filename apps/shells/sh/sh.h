#include <stdlib.h>
#include <string.h>

#if 0
#include <unistd.h>
#include <fcntl.h>
#endif

/* -------- sh.h -------- */
/*
 * shell
 */

#define	LINELIM	4096
#define	NPUSH	8	/* limit to input nesting */

#define	NOFILE	20	/* Number of open files */
#define	NUFILE	10	/* Number of user-accessible files */
#define	FDBASE	10	/* First file usable by Shell */

/*
 * values returned by wait
 */
#define	WAITSIG(s)  ((s)&0177)
#define	WAITVAL(s)  (((s)>>8)&0377)
#define	WAITCORE(s) (((s)&0200)!=0)


#define __STDC__
/*
 * library and system defintions
 */
#ifdef __STDC__
typedef void xint;	/* base type of jmp_buf, for not broken compilers */
#else
typedef char * xint;	/* base type of jmp_buf, for broken compilers */
#endif

/*
 * Description of a command or an operation on commands.
 * Might eventually use a union.
 */
struct op {
	int	type;			/* operation type, see below */
	char	**words;		/* arguments to a command    */
	struct	ioword	**ioact;	/* IO actions (eg, < > >>)   */
	struct op *left;
	struct op *right;
	char	*str;			/* identifier for case and for */
};

/*
 * shell components
 */
#include "area.h"
#include "word.h"
#include "io.h"
#include "var.h"

#define	QUOTE	0200

#define	NOBLOCK	((struct op *) NULL)
#define	NOWORD	((char *) NULL)
#define	NOWORDS	((char **) NULL)
#define	NOPIPE	((int *) NULL)


#define	TCOM	1	/* command */
#define	TPAREN	2	/* (c-list) */
#define	TPIPE	3	/* a | b */
#define	TLIST	4	/* a [&;] b */
#define	TOR	5	/* || */
#define	TAND	6	/* && */
#define	TFOR	7
#define	TDO	8
#define	TCASE	9
#define	TIF	10
#define	TWHILE	11
#define	TUNTIL	12
#define	TELIF	13
#define	TPAT	14	/* pattern in case */
#define	TBRACE	15	/* {c-list} */
#define	TASYNC	16	/* c & */

/*
 * actions determining the environment of a process
 */
#define	BIT(i)	(1<<(i))
#define	FEXEC	BIT(0)	/* execute without forking */

/*
 * flags to control evaluation of words
 */
#define	DOSUB	1	/* interpret $, `, and quotes */
#define	DOBLANK	2	/* perform blank interpretation */
#define	DOGLOB	4	/* interpret [?* */
#define	DOKEY	8	/* move words with `=' to 2nd arg. list */
#define	DOTRIM	16	/* trim resulting string */

#define	DOALL	(DOSUB|DOBLANK|DOGLOB|DOKEY|DOTRIM)

extern	char	**dolv;
extern	int	dolc;
extern	int	exstat;
extern  char	gflg;
extern  int	talking;	/* interactive (talking-type wireless) */
extern  int	execflg;
extern  int	multiline;	/* \n changed to ; */
extern  struct	op *outtree;	/* result from parser */

extern	xint	*failpt;
extern	xint	*errpt;

struct	brkcon {
	jmp_buf	brkpt;
	struct	brkcon	*nextlev;
};
extern	struct brkcon	*brklist;
extern	int	isbreak;

/*
 * redirection
 */
#define	IOREAD	1	/* < */
#define	IOHERE	2	/* << (here file) */
#define	IOWRITE	4	/* > */
#define	IOCAT	8	/* >> */
#define	IOXHERE	16	/* ${}, ` in << */
#define	IODUP	32	/* >&digit */
#define	IOCLOSE	64	/* >&- */

#define	IODEFAULT (-1)	/* token for default IO unit */

extern	struct	wdblock	*wdlist;
extern	struct	wdblock	*iolist;

/*
 * parsing & execution environment
 */
extern struct	env {
	char	*linep;
	struct	io *iobase;
	struct	io *iop;
	xint	*errpt;
	int	iofd;
	struct	env *oenv;
} e;

/*
 * flags:
 * -e: quit on error
 * -k: look for name=value everywhere on command line
 * -n: no execution
 * -t: exit after reading and executing one command
 * -v: echo as read
 * -x: trace
 * -u: unset variables net diagnostic
 */
extern	char *flag;

extern	char *null;		/* null value for variable */
extern	int  intr;		/* interrupt pending */

extern	char *trap[NSIGS+1];
extern	char ourtrap[NSIGS+1];
extern	int  trapset;		/* trap pending */

extern	int  heedint;		/* heed interrupt signals */

extern	int  yynerrs;		/* yacc */

extern	char line[LINELIM];
extern	char *elinep;

/*
 * other functions
 */
#ifdef __STDC__
int (*inbuilt(char *s))(void);
#else
int (*inbuilt())();
#endif
char *rexecve(char *c, char **v, char **envp);
char *space(int n);
char *strsave(char *s, int a);
char *evalstr(char *cp, int f);
char *putn(int n);
char *itoa(unsigned u, int n);
char *unquote(char *as);
struct var *lookup(char *n);
int  rlookup(char *n);
struct wdblock *glob(char *cp, struct wdblock *wb);
int  subgetc(char ec, int quoted);
char **makenv(void);
char **eval(char **ap, int f);
int  setstatus(int s);
int  waitfor(int lastpid, int canintr);

void onintr(int s); /* SIGINT handler */

int  newenv(int f);
void quitenv(void);
void err(char *s);
int  anys(char *s1, char *s2);
int  any(int c, char *s);
void next(int f);
void setdash(void);
void onecommand(void);
void runtrap(int i);
void xfree(char *s);
int  letter(int c);
int  digit(int c);
int  letnum(int c);
int  gmatch(char *s, char *p);

/*
 * error handling
 */
void leave(void);	/* abort shell (or fail in subshell) */
void fail(void);	/* fail but return to process next command */
void warn(char *s);
int  sig(int i);	/* default signal handler */
