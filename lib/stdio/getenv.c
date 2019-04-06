#include	<stdio.h>
#include	<string.h>

char **	environ = NULL;

char *
getenv(s)
char *	s;
{
	register char **xp;
	short i;

	i = strlen(s);
	for(xp = environ ; *xp ; xp++)
		if(strncmp(*xp, s, i) == 0 && (*xp)[i] == '=')
			return *xp + i+1;
	return (char *)0;
}
