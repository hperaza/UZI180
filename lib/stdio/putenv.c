/* Copyright (C) 1999 Greg Haerr <greg@censoft.com>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 *
 * Adapted to UZI180 by Hector Peraza.
 */

#include <string.h>
#include <stdlib.h>
#include <unix.h>

extern int  errno;
extern char **environ;		/* process global environment */

/*
 * Put or delete a string from the global process environment
 *
 * 'NAME=value'	adds environment variable name with value
 * 'NAME'	deletes environent variable if exists
 */

int putenv(char *var) 
{
    static char **putenv_environ = NULL;  /* ptr to any environment we allocated */
    char **env;
    int	 envp_count, envp_len, namelen, heap_bytes;
    char *nextstr, *rp;
    char **newenv, **nextarg;

    /* figure environment variable name length */
    if ((rp = strchr(var, '=')) == NULL)
	namelen = strlen(var);
    else
	namelen = rp - var;

    /* count environment bytes */

again:
    envp_len = 0;
    env = environ;
    while (*env) {
	/* check for variable in current environment */
	if (memcmp(var, *env, namelen) == 0 && 
	    (*env)[namelen] == '=') {

	    /* match, delete it and copy remaining up */
	    while ((env[0] = env[1]) != NULL) ++env;

	    /* if requested to delete, we're done */
	    if (rp == NULL) return 0;

	    goto again;
	}
	envp_len += strlen(*env++) + 1;
    }

    envp_len += strlen(var) + 1;	/* new environment variable */
    envp_count = env - environ + 2;	/* + 1 for NULL terminator  */
					/* + 1 for newly added var  */

    /* compute new environment allocation size */
    heap_bytes = envp_count * sizeof(char *) + envp_len;

    /* allocate new environment */
    if ((newenv = (char **) malloc(heap_bytes)) == NULL) {
	errno = ENOMEM;
	return -1;
    }

    /* build new environment */
    nextarg = newenv;
    nextstr = (char *) &newenv[envp_count];
    env = environ;
    while (*env) {
	*nextarg++ = nextstr;
	strcpy(nextstr, *env);
	nextstr += strlen(nextstr) + 1;
	++env;
    }

    /* add new variable */
    strcpy(nextstr, var);
    *nextarg++ = nextstr;
    *nextarg = NULL;

    /* free previous environment */
    if (putenv_environ) free(putenv_environ);

    /* set new global environment */
    environ = putenv_environ = newenv;

    return 0;
}
