/*
 *	termcap.h	V1.1	20/7/87		agc	Joypace Ltd
 *
 *	Copyright Joypace Ltd, London, UK, 1987. All rights reserved.
 *	This file may be freely distributed provided that this notice
 *	remains attached.
 *
 *	A public domain implementation of the termcap(3) routines.
 *
 *
 *
 *	 Klamer Schutte	      V1.2    Nov. 1988
 *
 *   - Can match multiple terminal names		 [tgetent]
 *   - Removal of **area assignments			 [tgetstr]
 *
 *	 Terrence W. Holm     V1.3    May, Sep, Oct.  1988
 *
 *   - Correct when TERM != name and TERMCAP is defined	 [tgetent]
 *   - Correct the comparison for the terminal name 	 [tgetent]
 *   - Correct the value of ^x escapes              	 [tgetstr]
 *   - Added %r to reverse row/column			 [tgoto]
 *   - Fixed end of definition test			 [tgetnum/flag/str]
 *
 *	 Terrence W. Holm     V1.4    Jan. 1989
 *
 *   - Incorporated Klamer's V1.2 fixes into V1.3
 *   - Added %d, (old %d is now %2)			 [tgoto]
 *   - Allow '#' comments in definition file		 [tgetent]
 */

#ifndef _TERMCAP_H
#define _TERMCAP_H

int  tgetent(char *_bp, char *_name);
int  tgetflag(char *_id);
int  tgetnum(char *_id);
char *tgetstr(char *_id, char **_area);
char *tgoto(char *_cm, int _destcol, int _destline);
/*int  tputs(char *_cp, int _affcnt, void (*_outc)(int _ch));*/

#endif /* _TERMCAP_H */
