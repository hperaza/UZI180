/* -------- area.h -------- */

/*
 * storage allocation
 */
char *getcell(unsigned nbytes);
void garbage(void);
void setarea(char *cp, int a);
int  getarea(char *cp);
void freearea(int a);
void freecell(char *cp);

extern	int	areanum;	/* current allocation area */

#define	NEW(type) (type *)getcell(sizeof(type))
#define	DELETE(obj)	freecell((char *)obj)
