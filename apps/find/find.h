/*######################## DEFINITIONS ##############################*/

#ifndef PATH_MAX
#define PATH_MAX 512
#endif

#ifdef S_IFLNK
#define LSTAT lstat
#else
#define LSTAT stat
#endif

#define SHELL "/bin/sh"
#define MAXARG          256	/* maximum length for an argv for -exec  */
#define BSIZE           512	/* POSIX wants 512 byte blocks           */
#define SECS_PER_DAY    (24L*60L*60L)	/* check your planet             */

#define OP_NAME          1	/* match name                            */
#define OP_PERM          2	/* check file permission bits            */
#define OP_TYPE          3	/* check file type bits                  */
#define OP_LINKS         4	/* check link count                      */
#define OP_USER          5	/* check owner                           */
#define OP_GROUP         6	/* check group ownership                 */
#define OP_SIZE          7	/* check size, blocks or bytes           */
#define OP_SIZEC         8	/* this is a fake for -size with 'c'     */
#define OP_INUM          9	/* compare inode number                  */
#define OP_ATIME        10	/* check last access time                */
#define OP_CTIME        11	/* check creation time                   */
#define OP_MTIME        12	/* check last modification time          */
#define OP_EXEC         13	/* execute command                       */
#define OP_OK           14	/* execute with confirmation             */
#define OP_PRINT        15	/* print name                            */
#define OP_PRINT0       16	/* print name null terminated            */
#define OP_NEWER        17	/* compare modification times            */
#define OP_AND          18	/* logical and (short circuit)           */
#define OP_OR           19	/* logical or (short circuit)            */
#define OP_XDEV         20	/* do not cross file-system boundaries   */
#define OP_DEPTH        21	/* descend directory before testing      */
#define OP_PRUNE        22	/* don't descend into current directory  */
#define OP_NOUSER       23	/* check validity of user id             */
#define OP_NOGROUP      24	/* check validity of group id            */
#define LPAR            25	/* left parenthesis                      */
#define RPAR            26	/* right parenthesis                     */
#define NOT             27	/* logical not                           */

/* Some return values: */
#define EOI             -1	/* end of expression                     */
#define NONE             0	/* not a valid predicate                 */

/* For -perm with symbolic modes: */
#define ISWHO(c)        ((c == 'u') || (c == 'g') || (c == 'o') || (c == 'a'))
#define ISOPER(c)       ((c == '-') || (c == '=') || (c == '+'))
#define ISMODE(c)       ((c == 'r') || (c == 'w') || (c == 'x') || \
			 (c == 's') || (c == 't'))
#define MUSER           1
#define MGROUP          2
#define MOTHERS         4


struct exec {
    int e_cnt;
    char *e_vec[MAXARG];
};

struct node {
    int n_type;			/* any OP_ or NOT */
    union {
	char *n_str;
	struct {
	    long n_val;
	    int n_sign;
	} n_int;
	struct exec *n_exec;
	struct {
	    struct node *n_left, *n_right;
	} n_opnd;
    } n_info;
};

struct oper {
    char *op_str;
    int op_val;
};

extern struct oper ops[];

extern char **ipp;		/* pointer to next argument during parsing       */
extern char *prog;		/* program name (== argv [0])                    */
extern char *epath;		/* value of PATH environment string              */
extern long current_time;	/* for computing age                             */
extern int tty;			/* fd for /dev/tty when using -ok                */
extern int xdev_flag;		/* cross device boundaries?                      */
extern int devnr;		/* device nr of first inode                      */
extern int depth_flag;		/* descend before check?                         */
extern int prune_here;		/* This is Baaaad! Don't ever do this again!     */
extern int um;			/* current umask()                               */
extern int needprint;		/* implicit -print needed?                       */


/* The prototypes: */
int  main(int argc, char **argv);
char *Malloc(int n);
char *Salloc(char *s);
void find(char *path, struct node * pred, char *last);
int  check(char *path, struct stat * st, struct node * n, char *last);
int  ichk(long val, struct node * n);
int  lex(char *str);
struct node *newnode(int t);
int  isnumber(char *str, int base, int sign);
void number(char *str, int base, long *pl, int *ps);
void fmode(char *str, long *pl, int *ps);
struct node *expr(int t);
struct node *primary(int t);
struct node *secondary(int t);
void checkarg(char *arg);
struct node *simple(int t);
void nonfatal(char *s1, char *s2);
void fatal(char *s1, char *s2);
int  smatch(char *s, char *t);
char *find_bin(char *s);
int  execute(int op, struct exec *e, char *path);
void domode(int op, int *mode, int bits);
