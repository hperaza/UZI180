/* -------- io.h -------- */

struct ioword {
	short	io_unit;	/* unit affected */
	short	io_flag;	/* action (below) */
	char	*io_name;	/* file name */
};

/* io buffer */
struct iobuf {
  unsigned id;				/* buffer id */
  char buf[512];			/* buffer */
  char *bufp;				/* pointer into buffer */
  char *ebufp;				/* pointer to end of buffer */
};

/* possible arguments to an IO function */
struct ioarg {
	char	*aword;
	char	**awordlist;
	int	afile;		/* file descriptor */
	unsigned afid;		/* buffer id */
	long	afpos;		/* file position */
	struct iobuf *afbuf;	/* buffer for this file */
};
extern struct ioarg ioargstack[NPUSH];

#define AFID_NOBUF	(~0)
#define AFID_ID		0

/* an input generator's state */
struct	io {
	int	(*iofn)();
	struct	ioarg	*argp;
	int	peekc;
	char	prev;		/* previous character read by readc() */
	char	nlcount;	/* for `'s */
	char	xchar;		/* for `'s */
	char	task;		/* reason for pushed IO */
};
extern	struct	io	iostack[NPUSH];

#define	XOTHER	0	/* none of the below */
#define	XDOLL	1	/* expanding ${} */
#define	XGRAVE	2	/* expanding `'s */
#define	XIO	3	/* file IO */

/* in substitution */
#define	INSUB()	(e.iop->task == XGRAVE || e.iop->task == XDOLL)

/*
 * input generators for IO structure
 */
int  nlchar(struct ioarg *ap);
int  strchar(struct ioarg *ap);
int  qstrchar(struct ioarg *ap);
int  filechar(struct ioarg *ap);
int  herechar(struct ioarg *ap);
int  linechar(struct ioarg *ap);
int  gravechar(struct ioarg *ap, struct io *iop);
int  qgravechar(struct ioarg *ap, struct io *iop);
int  dolchar(struct ioarg *ap);
int  wdchar(struct ioarg *ap);
void scraphere(void);
void freehere(int area);
void gethere(void);
void markhere(char *s, struct ioword *iop);
int  herein(char *hname, int xdoll);
int  run(struct ioarg *argp, int (*f)());

/*
 * IO functions
 */
int  eofc(void);
int  getc(int ec);
int  readc(void);
void unget(int c);
void ioecho(char c);
void prs(char *s);
void putc(char c);
void prn(unsigned u);
void closef(int i);
void closeall(void);

/*
 * IO control
 */
void pushio(struct ioarg *argp, int (*fn)());
int  remap(int fd);
int  openpipe(int *pv);
void closepipe(int *pv);
struct io *setbase(struct io *ip);

extern	struct	ioarg	temparg;	/* temporary for PUSHIO */
#define	PUSHIO(what,arg,gen) ((temparg.what = (arg)),pushio(&temparg,(gen)))
#define	RUN(what,arg,gen) ((temparg.what = (arg)), run(&temparg,(gen)))
