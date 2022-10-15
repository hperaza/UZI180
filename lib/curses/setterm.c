#include <curses.h>
#include "curspriv.h"

static void ttysetflags()
{
#ifdef UZI
  _tty.t_flags &= ~(LCASE | RAW | CBREAK | CRMOD | ECHO);

  if (_cursvar.rawmode) {
	_tty.t_flags |= RAW;
  }
  if (_cursvar.cbrkmode) {
	_tty.t_flags |= CBREAK;
  }
  if (_cursvar.echoit) {
	_tty.t_flags |= ECHO;
  }
  if (!NONL) {
	_tty.t_flags |= CRMOD;
  }
  ioctl(0, TIOCSETP, (char *) &_tty);
#else
  _tty.c_iflag |= ICRNL | IXON;
  _tty.c_oflag |= OPOST | ONLCR;
  _tty.c_lflag |= ECHO | ICANON | IEXTEN | ISIG;

  if (_cursvar.rawmode) {
	_tty.c_iflag &= ~(ICRNL | IXON);
	_tty.c_oflag &= ~(OPOST);
	_tty.c_lflag &= ~(ICANON | IEXTEN | ISIG);
  }
  if (_cursvar.cbrkmode) {
	_tty.c_lflag &= ~(ICANON);
  }
  if (!_cursvar.echoit) {
	_tty.c_lflag &= ~(ECHO | ECHONL);
  }
  if (NONL) {
	_tty.c_iflag &= ~(ICRNL);
	_tty.c_oflag &= ~(ONLCR);
  }
  tcsetattr(0, TCSANOW, &_tty);
#endif
}				/* ttysetflags */

void raw()
{
  _cursvar.rawmode = TRUE;
  ttysetflags();
}				/* raw */

void noraw()
{
  _cursvar.rawmode = FALSE;
  ttysetflags();
}				/* noraw */

void echo()
{
  _cursvar.echoit = TRUE;
  ttysetflags();
}

void noecho()
{
  _cursvar.echoit = FALSE;
  ttysetflags();
}

void nl()
{
  NONL = FALSE;
  ttysetflags();
}				/* nl */

void nonl()
{
  NONL = TRUE;
  ttysetflags();
}				/* nonl */

void cbreak()
{
  _cursvar.cbrkmode = TRUE;
  ttysetflags();
}				/* cbreak */

void nocbreak()
{
  _cursvar.cbrkmode = FALSE;
  ttysetflags();
}				/* nocbreak */
