#include <stdio.h>
#include <string.h>

#include "termcap.h"

extern char *capab;	/* the capability itself */

/*
 *	tputs - put the string cp out onto the terminal, using the function
 *	outc. This should do padding for the terminal, but I can't find a
 *	terminal that needs padding at the moment...
 */

int tputs(char *cp, int affcnt, void (*outc)(int ch))
{
    if (cp == (char *) NULL)
	return (1);
    /* Do any padding interpretation - left null for MINIX just now */
    while (*cp)
	(*outc) (*cp++);
    return (1);
}
