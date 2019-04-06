/* uud - bulletproof version of uudecode */

/*
 * Uud -- decode a uuencoded file back to binary form.
 *
 * UZI port of the minix version.
 */

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <unix.h>
#include <stdio.h>

#define loop	for(;;)

#define NCHARS  256
#define LINELEN 256
#define FILELEN 64
#define NORMLEN 60		/* allows for 80 encoded chars per line */

#define SEQMAX 'z'
#define SEQMIN 'a'
char seqc;
int first, secnd, check, numl;

FILE *inpf, *outf;
char *pos;
char ifname[FILELEN], ofname[FILELEN];
char *source = NULL, *target = NULL;
char blank, part = '\0';
int  partn, lens;
int  debug = 0, nochk = 0, onedone = 0;
int  chtbl[NCHARS], cdlen[NORMLEN + 3];

int  main(int argc, char **argv);
char *getnword(char *str, int n);
void gettable(void);
void decode(void);
void getfile(char *buf);
void format(char *fp, ...);
void doprnt(char *fp, char *ap);
void puti(unsigned int i, unsigned int r);

extern int access();

int main(argc, argv)
int argc;
char *argv[];
{
    int mode;
    register int i, j;
    char *curarg;
    char dest[FILELEN], buf[LINELEN];

    while ((curarg = argv[1]) != NULL && curarg[0] == '-') {
	if (((curarg[1] == 'd') || (curarg[1] == 'D')) &&
	    (curarg[2] == '\0')) {
	    debug = 1;
	} else if (((curarg[1] == 'n') || (curarg[1] == 'N')) &&
		   (curarg[2] == '\0')) {
	    nochk = 1;
	} else if (((curarg[1] == 't') || (curarg[1] == 'T')) &&
		   (curarg[2] == '\0')) {
	    argv++;
	    argc--;
	    if (argc < 2) {
		format("uud: Missing target directory.\n");
		exit(15);
	    }
	    target = argv[1];
	    if (debug)
		format("Target dir = %s\n", target);
	} else if (((curarg[1] == 's') || (curarg[1] == 'S')) &&
		    (curarg[2] == '\0')) {
	    argv++;
	    argc--;
	    if (argc < 2) {
		format("uud: Missing source directory.\n");
		exit(15);
	    }
	    source = argv[1];
	    if (debug)
		format("Source dir = %s\n", source);
	} else if (curarg[1] != '\0') {
	    format("Usage: uud [-n] [-d] [-s dir] [-t dir] [input-file]\n");
	    exit(1);
	} else
	    break;
	argv++;
	argc--;
    }

    if (curarg == NULL || ((curarg[0] == '-') && (curarg[1] == '\0'))) {
	inpf = stdin;
	strcpy(ifname, "<stdin>");
    } else {
	if (source != NULL) {
	    strcpy(ifname, source);
	    strcat(ifname, curarg);
	} else {
	    strcpy(ifname, curarg);
	}
	if ((inpf = fopen(ifname, "r")) == NULL) {
	    format("uud: Can't open %s\n", ifname);
	    exit(2);
	}
	numl = 0;
    }

    /* Set up the default translation table. */
    for (i = 0; i < ' '; i++) chtbl[i] = -1;
    for (i = ' ', j = 0; i < ' ' + 64; i++, j++) chtbl[i] = j;
    for (i = ' ' + 64; i < NCHARS; i++) chtbl[i] = -1;
    chtbl['`'] = chtbl[' '];	/* common mutation */
    chtbl['~'] = chtbl['^'];	/* an other common mutation */
    blank = ' ';

    /* Set up the line length table, to avoid computing lotsa * and / ...  */
    cdlen[0] = 1;
    for (i = 1, j = 5; i <= NORMLEN; i += 3, j += 4)
	cdlen[i] = (cdlen[i + 1] = (cdlen[i + 2] = j));

    /* Search for header or translation table line. */
    loop {		/* master loop for multiple decodes in one file */
	partn = 'a';
	loop {
	    if (fgets(buf, sizeof buf, inpf) == NULL) {
		if (onedone) {
		    if (debug)
			format("End of file.\n");
		    exit(0);
		} else {
		    format("uud: No begin line.\n");
		    exit(3);
		}
	    }
	    numl++;
	    if (strncmp(buf, "table", (size_t) 5) == 0) {
		gettable();
		continue;
	    }
	    if (strncmp(buf, "begin", (size_t) 5) == 0) {
		break;
	    }
	}
	lens = strlen(buf);
	if (lens) buf[--lens] = '\0';

	if (sscanf(buf, "begin%o%s", &mode, dest) != 2) {
	    format("uud: Missing filename in begin line.\n");
	    exit(10);
	}

	if (target != NULL) {
	    strcpy(ofname, target);
	    strcat(ofname, dest);
	} else {
	    strcpy(ofname, dest);
	}

	if ((outf = fopen(ofname, "w")) == NULL) {	/* binary! */
	    format("uud: Cannot open output file: %s\n", ofname);
	    exit(4);
	}

	if (debug)
	    format("Begin uudecoding: %s\n", ofname);

	seqc = SEQMAX;
	check = nochk ? 0 : 1;
	first = 1;
	secnd = 0;

	decode();
	fclose(outf);
	chmod(ofname, mode);

	onedone = 1;

	if (debug)
	    format("End uudecoding: %s\n", ofname);

    }		/* master loop for multiple decodes in one file */
}


/* Bring back a pointer to the start of the nth word. */

char *getnword(str, n)
register char *str;
register int n;
{
    while ((*str == '\t') || (*str == ' ')) str++;
    if (!*str) return NULL;

    while (--n) {
	while ((*str != '\t') && (*str != ' ') && (*str)) str++;
	if (!*str) return NULL;
	while ((*str == '\t') || (*str == ' ')) str++;
	if (!*str) return NULL;
    }

    return str;
}


/* Install the table in memory for later use. */

void gettable()
{
    register int c, n = 0;
    register char *cpt;
    char buf[LINELEN];

    for (c = 0; c < NCHARS; c++) chtbl[c] = -1;

  again:
    if (fgets(buf, sizeof buf, inpf) == NULL) {
	format("uud: EOF while in translation table.\n");
	exit(5);
    }
    numl++;
    if (strncmp(buf, "begin", (size_t) 5) == 0) {
	format("uud: Incomplete translation table.\n");
	exit(6);
    }
    cpt = buf + strlen(buf) - 1;
    *cpt = ' ';
    while (*(cpt) == ' ') {
	*cpt = 0;
	cpt--;
    }
    cpt = buf;
    while (c = *cpt) {
	if (chtbl[c] != -1) {
	    format("uud: Duplicate char in translation table.\n");
	    exit(7);
	}
	if (n == 0) blank = c;
	chtbl[c] = n++;
	if (n >= 64) return;
	cpt++;
    }
    goto again;
}


/* Copy from inpf to outf, decoding as you go along. */

void decode()
{
    register char *bp, *ut;
    register int *trtbl = chtbl;
    register int n, c, rlen;
    register unsigned int len;
    char buf[LINELEN], outl[LINELEN];

    loop {
	if (fgets(buf, sizeof buf, inpf) == NULL) {
	    format("uud: EOF before end.\n");
	    fclose(outf);
	    exit(8);
	}
	numl++;
	len = strlen(buf);
	if (len) buf[--len] = '\0';

        /* Is it an unprotected empty line before the end line ? */
	if (len == 0) continue;

	/* Get the binary line length. */
	n = trtbl[*buf];
	if (n >= 0)
	    goto decod;

	/* End of uuencoded file ? */
	if (strncmp(buf, "end", (size_t) 3) == 0)
	    return;

	/* End of current file ? : get next one. */
	if (strncmp(buf, "include", (size_t) 7) == 0) {
	    getfile(buf);
	    continue;
	}
	format("uud: Bad prefix line %d in file: %s\n", numl, ifname);
	if (debug)
	    format("Bad line = %s\n", buf);
	exit(11);

	/* Sequence checking ? */
    decod:
        rlen = cdlen[n];

	/* Is it the empty line before the end line ? */
	if (n == 0) continue;

	/* Pad with blanks. */
	for (bp = &buf[c = len]; c < rlen; c++, bp++) *bp = blank;

	/* Verify if asked for. */
	if (debug) {
	    for (len = 0, bp = buf; len < rlen; len++) {
		if (trtbl[*bp] < 0) {
		    format("Non uuencoded char <%c>, line %d in file: %s\n",
			   *bp, numl, ifname);
		    format("Bad line = %s\n", buf);
		    exit(16);
		}
		bp++;
	    }
	}

	/*
	 *  All this just to check for uuencodes that append
	 *  a 'z' to each line....
	 */
	if (secnd && check) {
	    secnd = 0;
	    if (buf[rlen] == SEQMAX) {
		check = 0;
		if (debug)
		    format("Sequence check turned off (2).\n");
	    } else if (debug)
		format("Sequence check on (2).\n");
	} else if (first && check) {
	    first = 0;
	    secnd = 1;
	    if (buf[rlen] != SEQMAX) {
		check = 0;
		if (debug)
		    format("No sequence check (1).\n");
	    } else if (debug)
		format("Sequence check on (1).\n");
	}

	/* There we check. */
	if (check) {
	    if (buf[rlen] != seqc) {
		format("uud: Wrong sequence line %d in %s\n",
		       numl, ifname);
		if (debug)
		    format("Sequence char is <%c> instead of <%c>.\n",
			   buf[rlen], seqc);
		exit(18);
	    }
	    seqc--;
	    if (seqc < SEQMIN) seqc = SEQMAX;
	}

	/*
	 * Output a group of 3 bytes (4 input characters). The input chars
	 * are pointed to by p, they are to be output to file f.n is used
	 * to tell us not to output all of them at the end of the file.
	 */
	ut = outl;
	len = n;
	bp = &buf[1];
	while (n > 0) {
	    *(ut++) = trtbl[*bp] << 2 | trtbl[bp[1]] >> 4;
	    n--;
	    if (n) {
		*(ut++) = (trtbl[bp[1]] << 4) | (trtbl[bp[2]] >> 2);
		n--;
	    }
	    if (n) {
		*(ut++) = trtbl[bp[2]] << 6 | trtbl[bp[3]];
		n--;
	    }
	    bp += 4;
	}
	if ((n = fwrite(outl, (size_t) 1, (size_t) len, outf)) <= 0) {
	    format("uud: Error on writing decoded file.\n");
	    exit(18);
	}
    }
}

/*
 * Find the next needed file, if existing, otherwise try further
 * on next file.
 */
void getfile(buf)
register char *buf;
{
    if ((pos = getnword(buf, 2)) == NULL) {
	format("uud: Missing include file name.\n");
	exit(17);
    } else if (source != NULL) {
	strcpy(ifname, source);
	strcat(ifname, pos);
    } else {
	strcpy(ifname, pos);
    }

    if (access(ifname, 04)) {
	if (debug) {
	    format("Can't find: %s\n", ifname);
	    format("Continuing to read same file.\n");
	}
    } else {
	if (freopen(ifname, "r", inpf) == inpf) {
	    numl = 0;
	    if (debug)
		format("Reading next section from: %s\n", ifname);
	} else {
	    format("uud: freopen abort: %s\n", ifname);
	    exit(9);
	}
    }

    loop {
	if (fgets(buf, LINELEN, inpf) == NULL) {
	    format("uud: No begin line after include: %s\n", ifname);
	    exit(12);
	}
	numl++;
	if (strncmp(buf, "table", (size_t) 5) == 0) {
	    gettable();
	    continue;
	}
	if (strncmp(buf, "begin", (size_t) 5) == 0)
	    break;
    }

    lens = strlen(buf);
    if (lens) buf[--lens] = '\0';

    /* Check the part suffix. */

    if ((pos = getnword(buf, 3)) == NULL) {
	format("uud: Missing part name, in included file: %s\n", ifname);
	exit(13);
    } else {
	part = *pos;
	partn++;
	if (partn > 'z') partn = 'a';
	if (part != partn) {
	    format("uud: Part suffix mismatch: <%c> instead of <%c>.\n",
		   part, partn);
	    exit(14);
	}
	if (debug)
	    format("Reading part %c\n", *pos);
    }
}

/*
 * Printf style formatting. (Borrowed from MicroEmacs by Dave Conroy.) 
 * A lot smaller than the full fledged printf.
 */
void format(char *fp, ...)
{
    va_list args;

    va_start(args, fp);
    doprnt(fp, args);
    va_end(args);
}


void doprnt(fp, ap)
register char *fp;
register char *ap;
{
    register int c, k;
    register char *s;

    while ((c = *fp++) != '\0') {
	if (c != '%') {
	    putchar(c);
	} else {
	    c = *fp++;
	    switch (c) {
	    case 'd':
		puti(*(int *) ap, 10);
		ap += sizeof(int);
		break;

	    case 's':
		s = *(char **) ap;
		while ((k = *s++) != '\0') putchar(k);
		ap += sizeof(char *);
		break;

	    case 'c':
		putchar(*(int *) ap);
		ap += sizeof(int);
		break;

	    default:
		putchar(c);
	    }
	}
    }
}


/* Put integer, in radix "r". */

void puti(i, r)
register unsigned int i;
register unsigned int r;
{
    register unsigned int q, s;

    if ((q = i / r) != 0)
	puti(q, r);

    s = i % r;

    if (s <= 9)
	putchar(s + '0');
    else
	putchar(s - 10 + 'A');
}
