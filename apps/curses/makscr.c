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


/* Makscr - make your own screen using arrow keys and space bar */
void makscr()
{
    int curx, cury;		/* current point on screen */
    char c;			/* input char */

    wclear(info);
    wmove(info, 0, 0);
    wprintw(info,
	    "Use arrow keys to move, space to place / erase, ^D to start",
	    NULL);
    wrefresh(info);
    curx = cury = 1;
    wmove(mns, cury - 1, curx - 1);
    wrefresh(mns);
    raw();
    noecho();
    for (;;) {
	c = wgetch(mns);
	if (c == '\004')
	    break;
	else if (c == ' ') {
	    if (present[cury][curx]) {
		--present[cury][curx];
		changes++;
		die++;
		mvwaddch(mns, cury, curx, ' ');
	    } else {
		++present[cury][curx];
		changes++;
		live++;
		mvwaddch(mns, cury, curx, '*');
	    }
	} else if (c == '\033') {
	    wgetch(mns);
	    switch (wgetch(mns)) {
	    case 'A':
		--cury;
		break;
	    case 'B':
		++cury;
		break;
	    case 'C':
		++curx;
		break;
	    case 'D':
		--curx;
		break;
	    default:
		break;
	    }
	}
	if (cury > maxrow) cury = minrow;
	if (cury < minrow) cury = maxrow;
	if (curx > maxcol) curx = mincol;
	if (curx < mincol) curx = maxcol;
	wmove(mns, cury, curx);
	wrefresh(mns);
    }
    noraw();
    wclear(info);
}


/* Update rules:  2 or 3 adjacent alive --- stay alive
 *                3 adjacent alive -- dead to live
 *                all else die or stay dead
 */
void update()
{				/* Does all mathmatical calculations */
    int  howmany, w, x, y, z;
    char c1, c2, c3, c4, c5, c6, c7, c8, c9;

    changes = die = live = 0;
    for (j = 0; j < maxrow; j++) {
	for (k = 0; k < maxcol; k++) {
	    w = j - 1;
	    x = j + 1;
	    y = k - 1;
	    z = k + 1;

	    c1 = past[w][y];
	    c2 = past[w][k];
	    c3 = past[w][z];
	    c4 = past[j][y];
	    c5 = past[j][k];
	    c6 = past[j][z];
	    c7 = past[x][y];
	    c8 = past[x][k];
	    c9 = past[x][z];
	    
	    if (w < 0) c1 = c2 = c3 = 0;
	    if (y < 0) c1 = c4 = c7 = 0;

	    howmany = c1 + c2 + c3 + c4 + c6 + c7 + c8 + c9;

	    switch (howmany) {
	    case 0:
	    case 1:
	    case 4:
	    case 5:
	    case 6:
	    case 7:
	    case 8:
		present[j][k] = 0;
		if (c5) changes++, die++;
		break;
	    case 3:
		present[j][k] = 1;
		if (!c5) changes++, live++;
		break;
	    default:
		break;
	    }
	}
    }
    if (live == die)
	++icnt;
    else
	icnt = 0;

    if (icnt == REPSTOP)
	cleanup(0);
}
