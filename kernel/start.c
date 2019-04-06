/***************************************************************
   UZI (Unix Z80 Implementation) Kernel:  start.c
----------------------------------------------------------------
 Adapted from UZI By Doug Braun, and UZI280 by Stefan Nitschke
            Copyright (C) 1998 by Harold F. Bower
       Portions Copyright (C) 1995 by Stefan Nitschke
****************************************************************/
/* Revisions:
 *  26.12.97 - added to resemble UZI280 layout.		     HFB
 */

#asm
	 psect  text
	global	_init0, _init    ; _init is PUBlic, _init0 is external

_init:
	jp	_init0
#endasm
