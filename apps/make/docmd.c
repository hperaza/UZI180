/*************************************************************************
 *
 *  m a k e :   m a k e . c
 *
 *  Do the actual making for make plus system dependent stuff
 *========================================================================
 * Edition history
 *
 *  #    Date                         Comments                       By
 * --- -------- ---------------------------------------------------- ---
 *   1    ??                                                         ??
 *   2 01.07.89 $<,$* bugs fixed                                     RAL
 *   3 23.08.89 (time_t)time((time_t*)0) bug fixed, N_EXISTS added   RAL
 *   4 30.08.89 leading sp. in cmd. output eliminated, indention ch. PSH,RAL
 *   5 03.09.89 :: time fixed, error output -> stderr, N_ERROR intr.
 *              fixed LZ elimintaed                                  RAL
 *   6 07.09.89 implmacro, DF macros,debug stuff added               RAL
 *   7 09.09.89 tos support added                                    PHH,RAL
 *   8 17.09.89 make1 arg. fixed, N_EXEC introduced                  RAL
 * ------------ Version 2.0 released ------------------------------- RAL
 *     18.05.90 fixed -n bug with silent rules.  (Now echos them.)   PAN
 *
 *************************************************************************/

#include <stdio.h>
#include <unistd.h>

#include "h.h"

extern char **environ;

#define execv(path, args) execve(path, args, environ);

extern bool execflag;

/*
 *	Exec a shell that returns exit status correctly (/bin/esh).
 *	The standard EON shell returns the process number of the last
 *	async command, used by the debugger (ugg).
 *	[exec on eon is like a fork+exec on unix]
 */
int dosh(char *string, char *shell)
{
#if 0
    return system(string);
#else
    int pid, exitc;
    char *argv[4];
    
    argv[0] = shell;
    argv[1] = "-c";
    argv[2] = string;
    argv[3] = 0;
    
    pid = fork();
    if (pid == -1) {
      return -1;
    } else if (pid == 0) {
      execve(argv[0], argv, environ);
      exit(255);
    } else {
      while (wait(&exitc) != pid) {}
      return exitc;
    }
#endif
}


/*
 *    Make a file look very outdated after an error trying to make it.
 *    Don't remove, this keeps hard links intact.  (kjb)
 */
int makeold(char *name)
{
    struct utimbuf a;

#ifdef uzi
    int utime();

    a.actime.t_time  = a.actime.t_date  = 0;
    a.modtime.t_time = a.modtime.t_date = 0;
#else
    a.actime = a.modtime = 0;	/* The epoch */
#endif
    return utime(name, &a);
}


void tellstatus(FILE *out, char *name, int status)
{
    char cwd[PATH_MAX];
    char *getcwd();

    fprintf(out, "%s in %s: ",
	    name, getcwd(cwd, sizeof(cwd)) == NULL ? "?" : cwd);

#ifdef uzi
    fprintf(out, "Exit code %d", (status >> 8) & 0xff);
#else
    if (WIFEXITED(status)) {
	fprintf(out, "Exit code %d", WEXITSTATUS(status));
    } else {
	fprintf(out, "Signal %d%s",
		WTERMSIG(status), status & 0x80 ? " - core dumped" : "");
    }
#endif
}


/*
 *	Get the modification time of a file.  If the first
 *	doesn't exist, it's modtime is set to 0.
 */
void modtime(struct name *np)
{
    struct stat info;
    int r;

    if (is_archive_ref(np->n_name)) {
	r = archive_stat(np->n_name, &info);
    } else {
	r = stat(np->n_name, &info);
    }
    if (r < 0) {
	if (errno != ENOENT)
	    fatal("Can't open %s: %s", np->n_name, errno);

	np->n_time = 0L;
	np->n_flag &= ~N_EXISTS;
    } else {
	np->n_time = convuzitime(&info.st_mtime);
	np->n_flag |= N_EXISTS;
    }
}


/*
 *	Update the mod time of a file to now.
 */
void touch(struct name *np)
{
    if (!domake || !silent)
	printf("touch(%s)\n", np->n_name);

    if (domake) {
        time_t now;
	struct utimbuf a;

        time(&now);

	a.actime = a.modtime = now;
	if (utime(np->n_name, &a) < 0)
	    printf("%s: '%s' not touched - non-existant\n",
		   myname, np->n_name);
    }
}


/*
 *	Do commands to make a target
 */
void docmds1(struct name *np, struct line *lp)
{
    register char *q;
    register char *p;
    register struct cmd *cp;
    bool ssilent;
    bool signore;
    int estat;
    char *shell;


    if (*(shell = getmacro("SHELL")) == '\0')
	shell = "/bin/sh";

    for (cp = lp->l_cmd; cp; cp = cp->c_next) {
	execflag = TRUE;
	strcpy(str1, cp->c_cmd);
	expmake = FALSE;
	expand(&str1s);
	q = str1;
	ssilent = silent;
	signore = ignore;
	while ((*q == '@') || (*q == '-')) {
	    if (*q == '@')	/*  Specific silent  */
		ssilent = TRUE;
	    else		/*  Specific ignore  */
		signore = TRUE;
	    if (!domake)
		putchar(*q);	/* Show all characters. */
	    q++;		/*  Not part of the command  */
	}

	for (p = q; *p; p++) {
	    if (*p == '\n' && p[1] != '\0') {
		*p = ' ';
		if (!ssilent || !domake)
		    fputs("\\\n", stdout);
	    } else if (!ssilent || !domake)
		putchar(*p);
	}
	if (!ssilent || !domake)
	    putchar('\n');

	if (domake || expmake) {	/*  Get the shell to execute it  */
	    fflush(stdout);
	    if ((estat = dosh(q, shell)) != 0) {
		if (estat == -1)
		    fatal("Couldn't execute %s", shell, 0);
		else if (signore) {
		    tellstatus(stdout, myname, estat);
		    printf(" (Ignored)\n");
		} else {
		    tellstatus(stderr, myname, estat);
		    fprintf(stderr, "\n");
		    if (!(np->n_flag & N_PREC))
#if 1
			if (makeold(np->n_name) == 0)
			    fprintf(stderr, "%s: made '%s' look old.\n",
				    myname, np->n_name);
#else
			if (unlink(np->n_name) == 0)
			    fprintf(stderr, "%s: '%s' removed.\n", myname,
				    np->n_name);
#endif
		    if (!conterr)
			exit(estat != 0);
		    np->n_flag |= N_ERROR;
		    return;
		}
	    }
	}
    }
}


void docmds(struct name *np)
{
    register struct line *lp;

    for (lp = np->n_line; lp; lp = lp->l_next)
	docmds1(np, lp);
}
