/* life - Conway's game of life		Author: Jim King */

/* clife.c - curses life simulator.  Translated from Pascal to C implementing
 *           curses Oct 1988 by pulsar@lsrhs, not jek5036@ritvax.isc.rit.edu
 *           life needs about 18kb stack space on MINIX.
 *
 * Flags:	-d  draw your own screen using arrows and space bar
 *		-p  print statistics on the bottom line during the game
 */

#include <curses.h>
#include <stdlib.h>
#include <unix.h>
#include <stdio.h>

#if __minix_vmd			/* Use a more random rand(). */
#define srand(seed)	srandom(seed)
#define rand()		random()
#endif

/* A value of -1 will make it go forever */
/* A value of 0 will make it exit immediately */
#define	REPSTOP		-1	/* number of repetitions before stop */

char present[23][80];		/* screen 1 cycle ago */
char past[23][80];		/* screen this cycle */
int  total;			/* total # of changes */
int  icnt;			/* counter to check for repetition */
int  maxrow = 22;		/* some defines to represent the screen */
int  maxcol = 79;
int  minrow = 0;
int  mincol = 0;
int  pri = 0;			/* flag for printing stats on bottom line */
int  draw = 0;			/* flag for drawing your own screen */
int  i, j, k;			/* loop counters */
int  cycle;			/* current cycle # */
int  changes;			/* # of changes this cycle (live + die) */
int  die;			/* number of deaths this cycle */
int  live;			/* number of births this cycle */

WINDOW *mns;			/* Main Screen */
WINDOW *info;			/* Bottom line */

void cleanup(int s);
void initialize(void);
void makscr(void);
void update(void);
void print(void);
int  main(int ac, char *av[]);

extern int getpid();

/* Cleanup - cleanup then exit */
void cleanup(s)
int s;
{
    move(23, 0);		/* go to bottom of screen */
    refresh();			/* update cursor */

    endwin();			/* shutdown curses */
    exit(1);			/* exit */
}

/* Initialize - init windows, variables, and signals */

void initialize()
{
    srand(getpid());		/* init random seed */

    initscr();			/* init curses */
    noecho();
    curs_set(0);

    signal(SIGINT, cleanup);	/* catch ^C */

    mns = newwin(maxrow, maxcol, 0, 0);	/* new window */
    scrollok(mns, FALSE);
    info = newwin(1, 80, 23, 0);
    scrollok(info, FALSE);
    wclear(mns);
    wclear(info);
    wmove(info, 0, 0);
    wrefresh(info);

    if (!draw) {		/* if no draw, make random pattern */
	for (j = 0; j < maxrow; j++) {
	    for (k = 0; k < maxcol; k++) {
		present[j][k] = rand() % 2;
		if (present[j][k] == 1)
		    changes++, live++;
	    }
	}
    }
}


/* Print - updates the screen according to changes from past to present */

void print()
{
    /* Updates the screen, greatly improved using curses */
    if (pri) {
	wmove(info, 0, 0);
	total += changes;
	cycle++;
	wprintw(info,
		"Cycle %5d | %5d changes: %5d died + %5d born = %5u total changes",
		cycle, changes, die, live, total);
	wclrtoeol(info);
    }
    for (j = 1; j < maxrow; j++) {
	for (k = 1; k < maxcol; k++) {
	    if (present[j][k] != past[j][k] && present[j][k] == 1) {
		wmove(mns, j, k);
		wprintw(mns, "@", NULL);
	    } else if (present[j][k] != past[j][k] && present[j][k] == 0) {
		wmove(mns, j, k);
		wprintw(mns, " ", NULL);
	    }
	}
    }
    if (pri) wrefresh(info);
    wrefresh(mns);
}
