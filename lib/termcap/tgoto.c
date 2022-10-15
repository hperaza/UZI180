#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "termcap.h"

extern char *capab;	/* the capability itself */

/*
 *	tgoto - given the cursor motion string cm, make up the string
 *	for the cursor to go to (destcol, destline), and return the string.
 *	Returns "OOPS" if something's gone wrong, or the string otherwise.
 */

char *tgoto(char *cm, int destcol, int destline)
{
    int incr = 0;
    int argno = 0;
    int numval;
    static char ret[24];
    char *rp = ret;

    for (; *cm; cm++) {
	if (*cm == '%') {
	    switch (*++cm) {
	    case 'i':
		incr = 1;
		break;

	    case 'r':
		argno = 1;
		break;

	    case '+':
		numval = (argno == 0 ? destline : destcol);
		*rp++ = numval + incr + *++cm;
		argno = 1 - argno;
		break;

	    case '2':
		numval = (argno == 0 ? destline : destcol);
		numval = (numval + incr) % 100;
		*rp++ = '0' + (numval / 10);
		*rp++ = '0' + (numval % 10);
		argno = 1 - argno;
		break;

	    case 'd':
		numval = (argno == 0 ? destline : destcol);
		numval = (numval + incr) % 1000;
		if (numval > 99)
		    *rp++ = '0' + (numval / 100);
		if (numval > 9)
		    *rp++ = '0' + (numval / 10) % 10;
		*rp++ = '0' + (numval % 10);
		argno = 1 - argno;
		break;

	    case '%':
		*rp++ = '%';
		break;

	    default:
		return ("OOPS");
	    }

	} else
	    *rp++ = *cm;
    }

    *rp = '\0';
    return (ret);
}
