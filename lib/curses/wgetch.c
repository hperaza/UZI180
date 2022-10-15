#include <curses.h>
#include <stdio.h>
#include "curspriv.h"

int wgetch(win)
WINDOW *win;
{
  bool weset = FALSE;
  int  inp;

  if (!win->_scroll && (win->_flags & _FULLWIN)
      && win->_curx == win->_maxx - 1 && win->_cury == win->_maxy - 1)
	return ERR;
  if (_cursvar.echoit && !_cursvar.rawmode) {
	cbreak();
	weset++;
  }
#ifdef UZI
  inp = 0;
  read(0, &inp, 1);
#else
  inp = getchar();
#endif
  if (_cursvar.echoit) {
	mvwaddch(curscr, win->_cury + win->_begy,
		 win->_curx + win->_begx, inp);
	waddch(win, inp);
  }
  if (weset) nocbreak();
  return inp;
}
