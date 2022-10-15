/*	archive.c - archive support			Author: Kees J. Bot
 *								13 Nov 1993
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "h.h"

long lseek(int, unsigned long, int);
int stat();
int open();

#define SEEK_SET 0


#define arraysize(a)	(sizeof(a) / sizeof((a)[0]))
#define arraylimit(a)	((a) + arraysize(a))

/* ASCII ar header. */

#define ASCII_ARMAG	"!<arch>\n"
#define ASCII_SARMAG	8
#define ASCII_ARFMAG	"`\n"

struct ascii_ar_hdr {
    char ar_name[16];
    char ar_date[12];
    char ar_uid[6];
    char ar_gid[6];
    char ar_mode[8];
    char ar_size[10];
    char ar_fmag[2];
};

/* ACK ar header. */

#define	ACK_ARMAG	0177545
#define ACK_AALMAG	0177454

struct ack_ar_hdr {
    char ar_name[14];
    unsigned long ar_date;
    unsigned char ar_uid;
    unsigned char ar_gid;
    unsigned short ar_mode;
    unsigned long ar_size;
};

typedef struct archname {
    struct archname *next;	/* Next on the hash chain. */
    char name[16];		/* One archive entry. */
    ulong date;			/* The timestamp. */
    /* (no need for other attibutes) */
} archname_t;

size_t namelen;		/* Max name length, 14 or 16. */

#define HASHSIZE	(64 << sizeof(int))

archname_t *nametab[HASHSIZE];

char *lpar, *rpar;	/* Leave these at '(' and ')'. */

static long ar_atol(char *s, size_t n);
static int hash(char *name);


/* True if name is of the form "archive(file)". */
int is_archive_ref(char *name)
{
    char *p = name;

    while (*p != 0 && *p != '(' && *p != ')')
	p++;
    lpar = p;
    if (*p++ != '(')
	return 0;

    while (*p != 0 && *p != '(' && *p != ')')
	p++;
    rpar = p;
    if (*p++ != ')')
	return 0;

    return *p == 0;
}

/* Compute a hash value out of a name. */
static int hash(char *name)
{
    unsigned h = 0;
    unsigned char *p = (unsigned char *) name;
    int n = namelen;

    while (*p != 0) {
	h = h * 0x1111 + *p++;
	if (--n == 0)
	    break;
    }

    return h % arraysize(nametab);
}

/* Enter a name to the table, or return the date of one already there. */
int searchtab(char *name, ulong *date, int scan)
{
    archname_t **pnp, *np;
    int cmp = 1;

    pnp = &nametab[hash(name)];

    while ((np = *pnp) != NULL
	   && (cmp = strncmp(name, np->name, namelen)) > 0) {
	pnp = &np->next;
    }

    if (cmp != 0) {
	if (scan) {
	    errno = ENOENT;
	    return -1;
	}
	if ((np = (archname_t *) malloc(sizeof(*np))) == NULL)
	    fatal("No memory for archive name cache", (char *) 0, 0);
	strncpy(np->name, name, namelen);
	np->date = *date;
	np->next = *pnp;
	*pnp = np;
    }
    if (scan)
	*date = np->date;
    return 0;
}

/* Delete the name cache, a different library is to be read. */
void deltab()
{
    archname_t **pnp, *np, *junk;

    for (pnp = nametab; pnp < arraylimit(nametab); pnp++) {
	for (np = *pnp; np != NULL;) {
	    junk = np;
	    np = np->next;
	    free(junk);
	}
	*pnp = NULL;
    }
}

/* Transform a string into a number.  Ignore the space padding. */
static long ar_atol(char *s, size_t n)
{
    long l = 0;

    while (n > 0) {
	if (*s != ' ')
	    l = l * 10 + (*s - '0');
	s++;
	n--;
    }
    return l;
}

/* Read a modern ASCII type archive. */
int read_ascii_archive(int afd)
{
    struct ascii_ar_hdr hdr;
    unsigned long pos = 8;
    char *p;
    ulong date;

    namelen = 16;

    for (;;) {
	if (lseek(afd, pos, SEEK_SET) == -1)
	    return -1;

	switch (read(afd, &hdr, sizeof(hdr))) {
	case sizeof(hdr):
	    break;
	case -1:
	    return -1;
	default:
	    return 0;
	}

	if (strncmp(hdr.ar_fmag, ASCII_ARFMAG, sizeof(hdr.ar_fmag)) != 0) {
	    errno = EINVAL;
	    return -1;
	}

	/* Strings are space padded! */
	for (p = hdr.ar_name; p < hdr.ar_name + sizeof(hdr.ar_name); p++) {
	    if (*p == ' ') {
		*p = 0;
		break;
	    }
	}

	/* Add a file to the cache. */
	date = ar_atol(hdr.ar_date, sizeof(hdr.ar_date));
	searchtab(hdr.ar_name, &date, 0);

	pos += sizeof(hdr) + ar_atol(hdr.ar_size, sizeof(hdr.ar_size));
	pos = (pos + 1) & (~(unsigned long) 1);
    }
}

/* Read an ACK type archive. */
int read_ack_archive(int afd)
{
    unsigned char raw_hdr[14 + 4 + 1 + 1 + 2 + 4];
    struct ack_ar_hdr hdr;
    unsigned long pos = 2;
    ulong date;

    namelen = 14;

    for (;;) {
	if (lseek(afd, pos, SEEK_SET) == -1)
	    return -1;

	switch (read(afd, raw_hdr, sizeof(raw_hdr))) {
	case sizeof(raw_hdr):
	    break;
	case -1:
	    return -1;
	default:
	    return 0;
	}

	/* Copy the useful fields from the raw bytes transforming PDP-11
	 * style numbers to native format.
	 */
	memcpy(hdr.ar_name, raw_hdr + 0, 14);
	hdr.ar_date = (long) raw_hdr[14 + 1] << 24
	    | (long) raw_hdr[14 + 0] << 16
	    | (long) raw_hdr[14 + 3] << 8 | (long) raw_hdr[14 + 2] << 0;
	hdr.ar_size = (long) raw_hdr[22 + 1] << 24
	    | (long) raw_hdr[22 + 0] << 16
	    | (long) raw_hdr[22 + 3] << 8 | (long) raw_hdr[22 + 2] << 0;

	/* Add a file to the cache. */
	date = hdr.ar_date;
	searchtab(hdr.ar_name, &date, 0);

	pos = (pos + 26 + hdr.ar_size + 1) & (~(unsigned long) 1);
    }
}
