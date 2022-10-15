/* -------- var.h -------- */

struct	var {
	char	*value;
	char	*name;
	struct	var	*next;
	char	status;
};

#define	COPYV	1	/* flag to setval, suggesting copy */
#define	RONLY	01	/* variable is read-only */
#define	EXPORT	02	/* variable is to be exported */
#define	GETCELL	04	/* name & value space was got with getcell */

extern	struct	var	*vlist;		/* dictionary */

extern	struct	var	*homedir;	/* home directory */
extern	struct	var	*prompt;	/* main prompt */
extern	struct	var	*cprompt;	/* continuation prompt */
extern	struct	var	*path;		/* search path for commands */
extern	struct	var	*shell;		/* shell to interpret command files */
extern	struct	var	*ifs;		/* field separators */

int  yyparse(void);
struct var *lookup(char *n);
void setval(struct var *vp, char *val);
void nameval(struct var *vp, char *val, char *name);
void export(struct var *vp);
void ronly(struct var *vp);
int  isassign(char *s);
int  checkname(char *cp);
int  assign(char *s, int cf);
void putvlist(int f, int out);
int  eqname(char *n1, char *n2);

int  execute(struct op *t, int *pin, int *pout, int act);
