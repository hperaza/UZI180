/* curses.h - defines macros and prototypes for curses */

#ifndef _CURSES_H
#define _CURSES_H

#include <termios.h>
#include <stdarg.h>
#include <stdio.h>

typedef int bool;

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef ERR
#define ERR (-1)	/* general error flag */
#endif
#ifndef OK
#define OK 0		/* general OK flag */
#endif

#ifdef UZI
#define const
#endif

/* Macros. */
#define box(win,vc,hc) wbox(win,0,0,0,0,vc,hc)
#define addch(ch) waddch(stdscr,ch)
#define mvaddch(y,x,ch) (wmove(stdscr,y,x)==ERR?ERR:waddch(stdscr,ch))
#define mvwaddch(win,y,x,ch) (wmove(win,y,x)==ERR?ERR:waddch(win,ch))
#define getch() wgetch(stdscr)
#define mvgetch(y,x) (wmove(stdscr,y,x)==ERR?ERR:wgetch(stdscr))
#define mvwgetch(win,y,x) (wmove(win,y,x)==ERR?ERR:wgetch(win))
#define addstr(str) waddstr(stdscr,str)
#define mvaddstr(y,x,str) (wmove(stdscr,y,x)==ERR?ERR:waddstr(stdscr,str))
#define mvwaddstr(win,y,x,str) (wmove(win,y,x)==ERR?ERR:waddstr(win,str))
#define getstr(str) wgetstr(stdscr,str)
#define mvgetstr(y,x,str) (wmove(stdscr,y,x)==ERR?ERR:wgetstr(stdscr,str))
#define mvwgetstr(win,y,x,str) (wmove(win,y,x)==ERR?ERR:wgetstr(win,str))
#define move(y,x) wmove(stdscr,y,x)
#define clear() wclear(stdscr)
#define erase() werase(stdscr)
#define clrtobot() wclrtobot(stdscr)
#define mvclrtobot(y,x) (wmove(stdscr,y,x)==ERR?ERR:wclrtobot(stdscr))
#define mvwclrtobot(win,y,x) (wmove(win,y,x)==ERR?ERR:wclrtobot(win))
#define clrtoeol() wclrtoeol(stdscr)
#define mvclrtoeol(y,x) (wmove(stdscr,y,x)==ERR?ERR:wclrtoeol(stdscr))
#define mvwclrtoeol(win,y,x) (wmove(win,y,x)==ERR?ERR:wclrtoeol(win))
#define insertln() winsertln(stdscr)
#define mvinsertln(y,x) (wmove(stdscr,y,x)==ERR?ERR:winsertln(stdscr))
#define mvwinsertln(win,y,x) (wmove(win,y,x)==ERR?ERR:winsertln(win))
#define deleteln() wdeleteln(stdscr)
#define mvdeleteln(y,x) (wmove(stdscr,y,x)==ERR?ERR:wdeleteln(stdscr))
#define mvwdeleteln(win,y,x) (wmove(win,y,x)==ERR?ERR:wdeleteln(win))
#define refresh() wrefresh(stdscr)
#define inch() winch(stdscr)
#define insch(ch) winsch(stdscr,ch)
#define mvinsch(y,x,ch) (wmove(stdscr,y,x)==ERR?ERR:winsch(stdscr,ch))
#define mvwinsch(win,y,x,ch) (wmove(win,y,x)==ERR?ERR:winsch(win,ch))
#define delch() wdelch(stdscr)
#define mvdelch(y,x) (wmove(stdscr,y,x)==ERR?ERR:wdelch(stdscr))
#define mvwdelch(win,y,x) (wmove(win,y,x)==ERR?ERR:wdelch(win))
#define standout() wstandout(stdscr)
#define wstandout(win) ((win)->_attrs |= A_STANDOUT)
#define standend() wstandend(stdscr)
#define wstandend(win) ((win)->_attrs &= ~A_STANDOUT)
#define attrset(attrs) wattrset(stdscr, attrs)
#define wattrset(win, attrs) ((win)->_attrs = (attrs))
#define attron(attrs) wattron(stdscr, attrs)
#define wattron(win, attrs) ((win)->_attrs |= (attrs))
#define attroff(attrs) wattroff(stdscr,attrs)
#define wattroff(win, attrs) ((win)->_attrs &= ~(attrs))
#define getyx(win,y,x) (y = (win)->_cury, x = (win)->_curx)
#ifdef UZI
#define resetty() ioctl(0, TIOCSETP, &_orig_tty)
#else
#define resetty() tcsetattr(1, TCSANOW, &_orig_tty)
#endif

/* Video attribute definitions. */
#define	A_BLINK        0x0100
#define	A_BLANK        0
#define	A_BOLD         0x0200
#define	A_DIM          0
#define	A_PROTECT      0
#define	A_REVERSE      0x0400
#define	A_STANDOUT     0x0800
#define	A_UNDERLINE    0x1000
#define	A_ALTCHARSET   0x2000

/* Type declarations. */
typedef struct {
  int	   _cury;			/* current pseudo-cursor */
  int	   _curx;
  int      _maxy;			/* max coordinates */
  int      _maxx;
  int      _begy;			/* origin on screen */
  int      _begx;
  int	   _flags;			/* window properties */
  int	   _attrs;			/* attributes of written characters */
  int      _tabsize;			/* tab character size */
  bool	   _clear;			/* causes clear at next refresh */
  bool	   _leave;			/* leaves cursor as it happens */
  bool	   _scroll;			/* allows window scrolling */
  bool	   _nodelay;			/* input character wait flag */
  bool	   _keypad;			/* flags keypad key mode active */
  int    **_line;			/* pointer to line pointer array */
  int	  *_minchng;			/* First changed character in line */
  int	  *_maxchng;			/* Last changed character in line */
  int	   _regtop;			/* Top/bottom of scrolling region */
  int	   _regbottom;
} WINDOW;

/* External variables */
extern int LINES;			/* terminal height */
extern int COLS;			/* terminal width */
extern bool NONL;			/* \n causes CR too ? */
extern WINDOW *curscr;			/* the current screen image */
extern WINDOW *stdscr;			/* the default screen window */
extern struct termios _orig_tty, _tty;

extern unsigned int ACS_ULCORNER;	/* terminal dependent block grafic */
extern unsigned int ACS_LLCORNER;	/* charcters.  Forget IBM, we are */
extern unsigned int ACS_URCORNER;	/* independent of their charset. :-) */
extern unsigned int ACS_LRCORNER;
extern unsigned int ACS_RTEE;
extern unsigned int ACS_LTEE;
extern unsigned int ACS_BTEE;
extern unsigned int ACS_TTEE;
extern unsigned int ACS_HLINE;
extern unsigned int ACS_VLINE;
extern unsigned int ACS_PLUS;
extern unsigned int ACS_S1;
extern unsigned int ACS_S9;
extern unsigned int ACS_DIAMOND;
extern unsigned int ACS_CKBOARD;
extern unsigned int ACS_DEGREE;
extern unsigned int ACS_PLMINUS;
extern unsigned int ACS_BULLET;
extern unsigned int ACS_LARROW;
extern unsigned int ACS_RARROW;
extern unsigned int ACS_DARROW;
extern unsigned int ACS_UARROW;
extern unsigned int ACS_BOARD;
extern unsigned int ACS_LANTERN;
extern unsigned int ACS_BLOCK;

char *unctrl(int _c);
int  baudrate(void);
void beep(void);
void cbreak(void);
void clearok(WINDOW *_win, bool _flag);
void clrscr(void);
void curs_set(int _visibility);
void delwin(WINDOW *_win);
void doupdate(void);
void echo(void);
int  endwin(void);
int  erasechar(void);
void fatal(char *_s);
int  fixterm(void);
void flash(void);
void gettmode(void);
void idlok(WINDOW *_win, bool _flag);
WINDOW *initscr(void);
void keypad(WINDOW *_win, bool _flag);
int  killchar(void);
void leaveok(WINDOW *_win, bool _flag);
char *longname(void);
void meta(WINDOW *_win, bool _flag);
int  mvcur(int _oldy, int _oldx, int _newy, int _newx);
int  mvinch(int _y, int _x);
int  mvprintw(int _y, int _x, const char *_fmt, ...);
int  mvscanw(int _y, int _x, const char *_fmt, ...);
int  mvwin(WINDOW *_win, int _begy, int _begx);
int  mvwinch(WINDOW *_win, int _y, int _x);
int  mvwprintw(WINDOW *_win, int _y, int _x, const char *_fmt, ...);
int  mvwscanw(WINDOW *_win, int _y, int _x, const char *_fmt, ...);
WINDOW *newwin(int _num_lines, int _num_cols, int _y, int _x);
void nl(void);
void nocbreak(void);
void nodelay(WINDOW *_win, bool _flag);
void noecho(void);
void nonl(void);
void noraw(void);
void outc(int _c);
void overlay(WINDOW *_win1, WINDOW *_win2);
void overwrite(WINDOW *_win1, WINDOW *_win2);
void poscur(int _r, int _c);
int  printw(const char *_fmt, ...);
void raw(void);
int  resetterm(void);
int  saveoldterm(void);
int  saveterm(void);
int  savetty(void);
int  scanw(const char *_fmt, ...);
void scroll(WINDOW *_win);
void scrollok(WINDOW *_win, bool _flag);
int  setscrreg(int _top, int _bottom);
int  setterm(char *_type);
int  setupterm(void);
WINDOW *subwin(WINDOW *_orig, int _nlines, int _ncols, int _y, int _x);
int  tabsize(int _ts);
void touchwin(WINDOW *_win);
int  waddch(WINDOW *_win, int _c);
int  waddstr(WINDOW *_win, char *_str);
int  wbox(WINDOW *_win, int _ymin, int _xmin, int _ymax,
	  int _xmax, unsigned int _v, unsigned int _h);
void wclear(WINDOW *_win);
int  wclrtobot(WINDOW *_win);
int  wclrtoeol(WINDOW *_win);
int  wdelch(WINDOW *_win);
int  wdeleteln(WINDOW *_win);
void werase(WINDOW *_win);
int  wgetch(WINDOW *_win);
int  wgetstr(WINDOW *_win, char *_str);
int  winch(WINDOW *_win);
int  winsch(WINDOW *_win, int _c);
int  winsertln(WINDOW *_win);
int  wmove(WINDOW *_win, int _y, int _x);
void wnoutrefresh(WINDOW *_win);
int  wprintw(WINDOW *_win, const char *_fmt, ...);
void wrefresh(WINDOW *_win);
int  wscanw(WINDOW *_win, const char *_fmt, ...);
int  wsetscrreg(WINDOW *_win, int _top, int _bottom);
int  wtabsize(WINDOW *_win, int _ts);

#endif /* _CURSES_H */
