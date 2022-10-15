#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "termcap.h"

extern char *capab;	/* the capability itself */

/*
 *	tgetstr - get the string capability corresponding to id and place
 *	it in area (advancing area at same time). Expand escape sequences
 *	etc. Returns the string, or NULL if it can't do it.
 */

char *tgetstr(char *id, char **area)
{
    register char *cp = capab;
    register char *wsp = *area;	/* workspace pointer  */

    if (cp == (char *) NULL || id == (char *) NULL)
	return ((char *) NULL);

    for (;;) {
	while (*cp++ != ':')
	    if (cp[-1] == '\0')
		return ((char *) NULL);

	while (isspace(*cp))
	    cp++;

	if (strncmp(cp, id, 2) == 0 && cp[2] == '=') {
	    for (cp += 3; *cp && *cp != ':'; wsp++, cp++)
		switch (*cp) {
		case '^':
		    *wsp = *++cp - '@';
		    break;

		case '\\':
		    switch (*++cp) {
		    case 'E':
			*wsp = '\033';
			break;
		    case 'n':
			*wsp = '\n';
			break;
		    case 'r':
			*wsp = '\r';
			break;
		    case 't':
			*wsp = '\t';
			break;
		    case 'b':
			*wsp = '\b';
			break;
		    case 'f':
			*wsp = '\f';
			break;
		    case '0':
		    case '1':
		    case '2':
		    case '3':
			{
			    int i;
			    int t = 0;
			    for (i = 0; i < 3 && isdigit(*cp); ++i, ++cp)
				t = t * 8 + *cp - '0';
			    *wsp = t;
			    cp--;
			    break;
			}
		    default:
			*wsp = *cp;
		    }
		    break;

		default:
		    *wsp = *cp;
		}

	    *wsp++ = '\0';

	    {
		char *ret = *area;
		*area = wsp;
		return (ret);
	    }
	}
    }				/* end for(;;) */
}
