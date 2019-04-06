/* chmod - Change file modes				Author: V. Archer */

/* Copyright 1991 by Vincent Archer
 *	You may freely redistribute this software, in source or binary
 *	form, provided that you do not alter this copyright mention in any
 *	way.
 */

#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define UZI
#ifdef UZI

#define S_ISDIR(x)	(((x) & F_MASK) == F_DIR)
#define S_ISCHR(x)	(((x) & F_MASK) == F_CDEV)
#define S_ISBLK(x)	(((x) & F_MASK) == F_BDEV)
#define S_ISDEV(x)	(S_ISBLK(x) || S_ISCHR(x))

#define S_IRUSR		OWN_RD
#define S_IWUSR		OWN_WR
#define S_IXUSR		OWN_EX

#define S_IRGRP		GRP_RD
#define S_IWGRP		GRP_WR
#define S_IXGRP		GRP_EX

#define S_IROTH		OTH_RD
#define S_IWOTH		OTH_WR
#define S_IXOTH		OTH_EX

#define S_ISUID		SET_UID
#define S_ISGID		SET_GID
#define S_ISVTX		SAV_TXT

#define S_IRWXU		(S_IRUSR|S_IWUSR|S_IXUSR)
#define S_IRWXG		(S_IRGRP|S_IWGRP|S_IXGRP)
#define S_IRWXO		(S_IROTH|S_IWOTH|S_IXOTH)

typedef unsigned int mode_t;

#define PATH_MAX	512

extern int umask(), stat(), chmod();

#define dirent direct

#endif

#ifndef S_ISLNK
#define S_ISLNK(mode)	0
#define lstat		stat
#endif

#define USR_MODES (S_ISUID|S_IRWXU)
#define GRP_MODES (S_ISGID|S_IRWXG)
#define EXE_MODES (S_IXUSR|S_IXGRP|S_IXOTH)
#ifdef S_ISVTX
#define ALL_MODES (USR_MODES|GRP_MODES|S_IRWXO|S_ISVTX)
#else
#define ALL_MODES (USR_MODES|GRP_MODES|S_IRWXO)
#endif


/* Common variables */
char *symbolic;
mode_t new_mode, u_mask;
int  rflag, errors;
struct stat st;
char path[PATH_MAX + 1];

int main(int argc, char **argv);
mode_t parsemode(char *symbolic, mode_t oldmode);
int do_change(char *name);
void usage(void);

/* Parse a P1003.2 4.7.7-conformant symbolic mode. */
mode_t parsemode(symbolic, oldmode)
char *symbolic;
mode_t oldmode;
{
    mode_t who, mask, newmode, tmpmask;
    char action;

    newmode = oldmode & ALL_MODES;
    while (*symbolic) {
	who = 0;
	for (; *symbolic; symbolic++) {
	    if (*symbolic == 'a') {
		who |= ALL_MODES;
		continue;
	    }
	    if (*symbolic == 'u') {
		who |= USR_MODES;
		continue;
	    }
	    if (*symbolic == 'g') {
		who |= GRP_MODES;
		continue;
	    }
	    if (*symbolic == 'o') {
		who |= S_IRWXO;
		continue;
	    }
	    break;
	}
	if (!*symbolic || *symbolic == ',')
	    usage();
	while (*symbolic) {
	    if (*symbolic == ',')
		break;
	    switch (*symbolic) {
		default: usage();
		case '+':
		case '-':
		case '=': action = *symbolic++; break;
	    }
	    mask = 0;
	    for (; *symbolic; symbolic++) {
		if (*symbolic == 'u') {
		    tmpmask = newmode & S_IRWXU;
		    mask |= tmpmask | (tmpmask << 3) | (tmpmask << 6);
		    symbolic++;
		    break;
		}
		if (*symbolic == 'g') {
		    tmpmask = newmode & S_IRWXG;
		    mask |= tmpmask | (tmpmask >> 3) | (tmpmask << 3);
		    symbolic++;
		    break;
		}
		if (*symbolic == 'o') {
		    tmpmask = newmode & S_IRWXO;
		    mask |= tmpmask | (tmpmask >> 3) | (tmpmask >> 6);
		    symbolic++;
		    break;
		}
		if (*symbolic == 'r') {
		    mask |= S_IRUSR | S_IRGRP | S_IROTH;
		    continue;
		}
		if (*symbolic == 'w') {
		    mask |= S_IWUSR | S_IWGRP | S_IWOTH;
		    continue;
		}
		if (*symbolic == 'x') {
		    mask |= EXE_MODES;
		    continue;
		}
		if (*symbolic == 's') {
		    mask |= S_ISUID | S_ISGID;
		    continue;
		}
		if (*symbolic == 'X') {
		    if (S_ISDIR(oldmode) || (oldmode & EXE_MODES))
			mask |= EXE_MODES;
		    continue;
		}
#ifdef S_ISVTX
		if (*symbolic == 't') {
		    mask |= S_ISVTX;
		    who |= S_ISVTX;
		    continue;
		}
#endif
		break;
	    }
	    switch (action) {
	    case '=':
		if (who)
		    newmode &= ~who;
		else
		    newmode = 0;
	    case '+':
		if (who)
		    newmode |= who & mask;
		else
		    newmode |= mask & (~u_mask);
		break;
	    case '-':
		if (who)
		    newmode &= ~(who & mask);
		else
		    newmode &= ~mask | u_mask;
	    }
	}
	if (*symbolic)
	    symbolic++;
    }
    return (newmode);
}


/*
 *  Main module. The single option possible (-R) does not warrant a call to
 *  the getopt() stuff.
 */

int main(argc, argv)
int argc;
char *argv[];
{
    int ex_code = 0;

    argc--;
    argv++;

    if (argc && strcmp(*argv, "-R") == 0) {
	argc--;
	argv++;
	rflag = 1;
    } else {
	rflag = 0;
    }

    if (!argc--) usage();

    if (!strcmp(argv[0], "--")) {	/* Allow chmod -- -r, as in Draft11 example */
	if (!argc--) usage();
	argv++;
    }

    symbolic = *argv++;
    if (!argc) usage();

    if (*symbolic >= '0' && *symbolic <= '7') {
	new_mode = 0;
	while (*symbolic >= '0' && *symbolic <= '7')
	    new_mode = (new_mode << 3) | (*symbolic++ & 07);
	if (*symbolic) usage();
	new_mode &= ALL_MODES;
	symbolic = (char *) 0;
    } else {
	u_mask = umask(0);
    }

    while (argc--)
	if (do_change(*argv++))
	    ex_code = 1;

    return (ex_code);
}


/* Apply a mode change to a given file system element. */
int do_change(name)
char *name;
{
    mode_t m;
    DIR *dirp;
    struct dirent *entp;
    char *namp;

    if (lstat(name, &st)) {
	perror(name);
	return (1);
    }

    if (S_ISLNK(st.st_mode) && rflag)
	return (0);		/* Note: violates POSIX. */

    if (!symbolic)
	m = new_mode;
    else
	m = parsemode(symbolic, st.st_mode);

    if (chmod(name, m)) {
	perror(name);
	errors = 1;
    } else {
	errors = 0;
    }

    if (S_ISDIR(st.st_mode) && rflag) {
	if (!(dirp = opendir(name))) {
	    perror(name);
	    return (1);
	}
	if (name != path)
	    strcpy(path, name);
	namp = path + strlen(path);
	*namp++ = '/';
	while (entp = readdir(dirp))
	    if (entp->d_name[0] != '.' ||
		(entp->d_name[1] &&
		 (entp->d_name[1] != '.' || entp->d_name[2]))) {
		strcpy(namp, entp->d_name);
		errors |= do_change(path);
	    }
	closedir(dirp);
	*--namp = '\0';
    }
    return (errors);
}


/* Display Posix prototype */
void usage()
{
    fprintf(stderr, "Usage: chmod [-R] mode file...\n");
    exit(1);
}
