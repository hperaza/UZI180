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
#include <stdlib.h>
#include <string.h>

#include "h.h"


extern char *ctime();

void make1(struct name *np, struct line *lp, register struct depend *qdp,
           char *basename, char *inputname)
{
    register struct depend *dp;


    if (dotouch)
	touch(np);
    else if (!(np->n_flag & N_ERROR)) {
	strcpy(str1, "");

	if (!inputname) {
	    inputname = str1;	/* default */
	    if (ambigmac)
		implmacros(np, lp, &basename, &inputname);
	}
	setDFmacro("<", inputname);

	if (!basename)
	    basename = str1;
	setDFmacro("*", basename);

	for (dp = qdp; dp; dp = qdp) {
	    if (strlen(str1))
		strcat(str1, " ");
	    strcat(str1, dp->d_name->n_name);
	    qdp = dp->d_next;
	    free(dp);
	}
	setmacro("?", str1);
	setDFmacro("@", np->n_name);

	if (lp)			/* lp set if doing a :: rule */
	    docmds1(np, lp);
	else
	    docmds(np);
    }
}

void implmacros(struct name *np, struct line *lp,
                char **pbasename, char **pinputname)
/* pbasename - Name without suffix  */
{
    struct line *llp;
    register char *p;
    register char *q;
    register char *suff;	/*  Old suffix  */
    int baselen;
    struct depend *dp;
    bool dpflag = FALSE;

    /* get basename out of target name */
    p = str2;
    q = np->n_name;
    suff = suffix(q);
    while (*q && (q < suff || !suff))
	*p++ = *q++;
    *p = '\0';
    if ((*pbasename = (char *) malloc(strlen(str2) + 1)) == (char *) 0)
	fatal("No memory for basename", (char *) 0, 0);
    strcpy(*pbasename, str2);
    baselen = strlen(str2);

    if (lp)
	llp = lp;
    else
	llp = np->n_line;

    while (llp) {
	for (dp = llp->l_dep; dp; dp = dp->d_next) {
	    if (strncmp(*pbasename, dp->d_name->n_name, baselen) == 0) {
		*pinputname = dp->d_name->n_name;
		return;
	    }
	    if (!dpflag) {
		*pinputname = dp->d_name->n_name;
		dpflag = TRUE;
	    }
	}
	if (lp)
	    break;
	llp = llp->l_next;
    }

#if NO_WE_DO_WANT_THIS_BASENAME
    free(*pbasename);		/* basename ambiguous or no dependency file */
    *pbasename = (char *) 0;
#endif
    return;
}

void dbgprint(int level, struct name *np, char *comment)
{
    char *timep;

    if (np) {
	timep = ctime(&np->n_time);
	timep[24] = '\0';
	fputs(&timep[4], stdout);
    } else
	fputs("                    ", stdout);
    fputs("   ", stdout);
    while (level--)
	fputs("  ", stdout);
    if (np) {
	fputs(np->n_name, stdout);
	if (np->n_flag & N_DOUBLE)
	    fputs("  :: ", stdout);
	else
	    fputs("  : ", stdout);
    }
    fputs(comment, stdout);
    putchar((int) '\n');
    fflush(stdout);
    return;
}
