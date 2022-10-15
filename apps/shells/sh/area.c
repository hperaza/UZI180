#include <unix.h>

int sbrk();

/* -------- area.c -------- */

#define	REGSIZE		sizeof(struct region)
#define GROWBY		256
#undef	SHRINKBY	64
#define FREE		32767
#define BUSY		0
#define	ALIGN		(sizeof(int)-1)

#include "area.h"

struct region {
    struct region *next;
    int area;
};

/*
 * All memory between (char *)areabot and (char *)(areatop+1) is
 * exclusively administered by the area management routines.
 * It is assumed that sbrk() and brk() manipulate the high end.
 */
static struct region *areabot;	/* bottom of area */
static struct region *areatop;	/* top of area */
static struct region *areanxt;	/* starting point of scan */

void initarea()
{
    while ((int) sbrk(0) & ALIGN) sbrk(1);

    areabot = (struct region *) sbrk(REGSIZE);
    areabot->next = areabot;
    areabot->area = BUSY;
    areatop = areabot;
    areanxt = areabot;
}

char *getcell(nbytes)
unsigned nbytes;
{
    register int nregio;
    register struct region *p, *q;
    register i;

    if (nbytes == 0)
	abort();		/* silly and defeats the algorithm */
    /*
     * round upwards and add administration area
     */
    nregio = (nbytes + (REGSIZE - 1)) / REGSIZE + 1;
    for (p = areanxt;;) {
	if (p->area > areanum) {
	    /*
	     * merge free cells
	     */
	    while ((q = p->next)->area > areanum && q != areanxt)
		p->next = q->next;
	    /*
	     * exit loop if cell big enough
	     */
	    if (q >= p + nregio)
		goto found;
	}
	p = p->next;
	if (p == areanxt)
	    break;
    }
    i = nregio >= GROWBY ? nregio : GROWBY;
    p = (struct region *) sbrk(i * REGSIZE);
    if (p == (struct region *) -1)
	return ((char *) NULL);
    p--;
    if (p != areatop)
	abort();		/* allocated areas are contiguous */
    q = p + i;
    p->next = q;
    p->area = FREE;
    q->next = areabot;
    q->area = BUSY;
    areatop = q;

  found:
    /*
     * we found a FREE area big enough, pointed to by 'p', and up to 'q'
     */
    areanxt = p + nregio;
    if (areanxt < q) {
	/*
	 * split into requested area and rest
	 */
	if (areanxt + 1 > q)
	    abort();		/* insufficient space left for admin */
	areanxt->next = q;
	areanxt->area = FREE;
	p->next = areanxt;
    }
    p->area = areanum;
    return ((char *) (p + 1));
}

void freecell(cp)
char *cp;
{
    register struct region *p;

    if ((p = (struct region *) cp) != NULL) {
	p--;
	if (p < areanxt) areanxt = p;
	p->area = FREE;
    }
}

void freearea(a)
register int a;
{
    register struct region *p, *top;

    top = areatop;
    for (p = areabot; p != top; p = p->next)
	if (p->area >= a)
	    p->area = FREE;
}

void setarea(cp, a)
char *cp;
int a;
{
    register struct region *p;

    if ((p = (struct region *) cp) != NULL) 
	(p - 1)->area = a;
}

int getarea(cp)
char *cp;
{
    return ((struct region *) cp - 1)->area;
}

void garbage()
{
    register struct region *p, *q, *top;

    top = areatop;
    for (p = areabot; p != top; p = p->next) {
	if (p->area > areanum) {
	    while ((q = p->next)->area > areanum)
		p->next = q->next;
	    areanxt = p;
	}
    }
#ifdef SHRINKBY
    if (areatop >= q + SHRINKBY && q->area > areanum) {
	brk((char *) (q + 1));
	q->next = areabot;
	q->area = BUSY;
	areatop = q;
    }
#endif
}
