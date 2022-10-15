#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "termcap.h"

char *capab = (char *) NULL;	/* the capability itself */

#if 0
/*  The following are not yet used.  */
extern short ospeed;		/* output speed */
extern char PC;			/* padding character */
extern char *BC;		/* back cursor movement */
extern char *UP;		/* up cursor movement */
#endif

/*
 *	tgetent - get the termcap entry for terminal name, and put it
 *	in bp (which must be an array of 1024 chars). Returns 1 if
 *	termcap entry found, 0 if not found, and -1 if file not found.
 */

int tgetent(char *bp, char *name)
{
    FILE *fp;
    char *file;
    char *term;
    short len = strlen(name);

    capab = bp;

    /* If TERMCAP begins with a '/' then use TERMCAP as the path   */
    /* Name of the termcap definitions file. If TERMCAP is a       */
    /* Definition and TERM equals "name" then use TERMCAP as the   */
    /* Definition. Otherwise use "/etc/termcap" as the path name.  */

    if ((file = getenv("TERMCAP")) == (char *) NULL)
	file = "/etc/termcap";
    else if (*file != '/')
	if ((term = getenv("TERM")) != (char *) NULL
	    && strcmp(term, name) == 0) {
	    *bp = '\0';
	    strncat(bp, file, 1023);
	    return (1);
	} else
	    file = "/etc/termcap";

    if ((fp = fopen(file, "r")) == (FILE *) NULL) {
	capab = (char *) NULL;	/* no valid termcap  */
	return (-1);
    }
    for (;;) {
	/* Read in each definition */
	int def_len = 0;
	char *cp = bp;

	do {
	    if (fgets(&bp[def_len], (unsigned int) (1024 - def_len), fp) ==
		(char *) NULL) {
		fclose(fp);
		capab = (char *) NULL;	/* no valid termcap */
		return (0);
	    }
	    def_len = strlen(bp) - 2;
	} while (bp[def_len] == '\\');

	while (isspace(*cp))
	    cp++;

	/* Comment lines start with a '#'  */
	if (*cp == '#')
	    continue;

	/* See if any of the terminal names in this definition */
	/* Match "name".                                       */

	do {
	    if (strncmp(name, cp, len) == 0 &&
		(cp[len] == '|' || cp[len] == ':')) {
		fclose(fp);
		return (1);
	    }
	    while ((*cp) && (*cp != '|') && (*cp != ':'))
		cp++;
	} while (*cp++ == '|');
    }
}
