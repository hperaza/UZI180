#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "termcap.h"

extern char *capab;	/* the capability itself */

/*
 *	tgetnum - get the numeric terminal capability corresponding
 *	to id. Returns the value, -1 if invalid.
 */

int tgetnum(char *id)
{
    register char *cp = capab;

    if (cp == (char *) NULL || id == (char *) NULL)
	return (-1);

    for (;;) {
	while (*cp++ != ':')
	    if (cp[-1] == '\0')
		return (-1);

	while (isspace(*cp))
	    cp++;

	if (strncmp(cp, id, 2) == 0 && cp[2] == '#')
	    return (atoi(cp + 3));
    }
}
