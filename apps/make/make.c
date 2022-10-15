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

#define sleep(x)  alarm(x);pause();

int  time_zone = 0;

bool execflag;

/*
 *	Recursive routine to make a target.
 */
int make(struct name *np, int level)
{
    register struct depend *dp;
    register struct line *lp;
    register struct depend *qdp;
    time_t now;
    ulong tnow, t, dtime = 0;
    bool dbgfirst = TRUE;
    char *basename = (char *) 0;
    char *inputname = (char *) 0;

    if (np->n_flag & N_DONE) {
	if (dbginfo)
	    dbgprint(level, np, "already done");
	return 0;
    }

    modtime(np);		/*  Gets modtime of this file  */

    for (;;) {
        time(&now);
        tnow = convuzitime(&now);
        if (tnow != np->n_time) break;
	/* Time of target is equal to the current time.  This bothers us, because
	 * we can't tell if it needs to be updated if we update a file it depends
	 * on within a second.  So wait a second.  (A per-second timer is too
	 * coarse for today's fast machines.)
	 */
	sleep(1);
    }

    if (rules) {
	for (lp = np->n_line; lp; lp = lp->l_next)
	    if (lp->l_cmd)
		break;
	if (!lp)
	    dyndep(np, &basename, &inputname);
    }

    if (!(np->n_flag & (N_TARG | N_EXISTS))) {
	fprintf(stderr, "%s: Don't know how to make %s\n", myname,
		np->n_name);
	if (conterr) {
	    np->n_flag |= N_ERROR;
	    if (dbginfo)
		dbgprint(level, np, "don't know how to make");
	    return 0;
	} else
	    exit(1);
    }

    for (qdp = (struct depend *) 0, lp = np->n_line; lp; lp = lp->l_next) {
	for (dp = lp->l_dep; dp; dp = dp->d_next) {
	    if (dbginfo && dbgfirst) {
		dbgprint(level, np, " {");
		dbgfirst = FALSE;
	    }
	    make(dp->d_name, level + 1);
	    if (np->n_time < dp->d_name->n_time)
		qdp = newdep(dp->d_name, qdp);
	    dtime = max(dtime, dp->d_name->n_time);
	    if (dp->d_name->n_flag & N_ERROR)
		np->n_flag |= N_ERROR;
	    if (dp->d_name->n_flag & N_EXEC)
		np->n_flag |= N_EXEC;
	}
	if (!quest && (np->n_flag & N_DOUBLE) &&
	    (np->n_time < dtime || !(np->n_flag & N_EXISTS))) {
	    execflag = FALSE;
	    make1(np, lp, qdp, basename, inputname);	/* free()'s qdp */
	    dtime = 0;
	    qdp = (struct depend *) 0;
	    if (execflag)
		np->n_flag |= N_EXEC;
	}
    }

    np->n_flag |= N_DONE;

    if (quest) {
	t = np->n_time;
	np->n_time = tnow;
	return (t < dtime);
    } else if ((np->n_time < dtime || !(np->n_flag & N_EXISTS))
	       && !(np->n_flag & N_DOUBLE)) {
	execflag = FALSE;
	make1(np, (struct line *) 0, qdp, basename, inputname);	/* free()'s qdp */
	np->n_time = tnow;
	if (execflag)
	    np->n_flag |= N_EXEC;
    } else if (np->n_flag & N_EXEC) {
	np->n_time = tnow;
    }

    if (dbginfo) {
	if (dbgfirst) {
	    if (np->n_flag & N_ERROR)
		dbgprint(level, np, "skipped because of error");
	    else if (np->n_flag & N_EXEC)
		dbgprint(level, np, "successfully made");
	    else
		dbgprint(level, np, "is up to date");
	} else {
	    if (np->n_flag & N_ERROR)
		dbgprint(level, (struct name *) 0,
			 "} skipped because of error");
	    else if (np->n_flag & N_EXEC)
		dbgprint(level, (struct name *) 0, "} successfully made");
	    else
		dbgprint(level, (struct name *) 0, "} is up to date");
	}
    }
    if (level == 0 && !(np->n_flag & N_EXEC))
	printf("%s: '%s' is up to date\n", myname, np->n_name);

    if (basename)
	free(basename);
    return 0;
}
