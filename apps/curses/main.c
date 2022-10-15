#include <curses.h>
#include <unix.h>
#include <stdio.h>

/* A value of -1 will make it go forever */
/* A value of 0 will make it exit immediately */
#define REPSTOP		-1	/* number of repetitions before stop */

extern char present[23][80];	/* screen 1 cycle ago */
extern char past[23][80];	/* screen this cycle */
extern int  total;		/* total # of changes */
extern int  icnt;		/* counter to check for repetition */
extern int  maxrow;		/* some defines to represent the screen */
extern int  maxcol;
extern int  minrow;
extern int  mincol;
extern int  pri;		/* flag for printing stats on bottom line */
extern int  draw;		/* flag for drawing your own screen */
extern int  i, j, k;		/* loop counters */
extern int  cycle;		/* current cycle # */
extern int  changes;		/* # of changes this cycle (live + die) */
extern int  die;		/* number of deaths this cycle */
extern int  live;		/* number of births this cycle */

extern WINDOW *mns;		/* Main Screen */
extern WINDOW *info;		/* Bottom line */


/* Main - main procedure */
int main(ac, av)
int ac;
char *av[];
{
    if (ac > 1) {
	for (j = 1; j < ac; j++) {
	    switch (av[j][1]) {
	    case 'd':
		++draw;
		break;
	    case 'p':
		++pri;
		break;
	    default:
		fprintf(stderr, "%s: usage: %s [-d] [-p]\n", av[0], av[0]);
		exit(1);
	    }
	}
    }

    initialize();
    if (draw)
	makscr();

    for (;;) {
	print();
#if 0
	for (j = 0; j < maxrow; j++) {
	    for (k = 0; k < maxcol; k++)
		past[j][k] = present[j][k];
	}
#else
	memcpy(past, present, 23 * 80 * sizeof(char));
#endif
	update();
    }
}
