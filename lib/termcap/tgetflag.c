#include <ctype.h>
#include <string.h>

#include "termcap.h"

extern char *capab;	/* the capability itself */

int tgetflag(char *id)
{
    register char *cp = capab;

    if (cp == (char *) NULL || id == (char *) NULL)
	return (-1);

    for (;;) {
	while (*cp++ != ':')
	    if (cp[-1] == '\0')
		return (0);

	while (isspace(*cp))
	    cp++;

	if (strncmp(cp, id, 2) == 0)
	    return (1);
    }
}
