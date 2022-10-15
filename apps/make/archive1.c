/*	archive.c - archive support			Author: Kees J. Bot
 *								13 Nov 1993
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "h.h"

typedef int16 dev_t;
typedef int16 ino_t;

extern char *lpar, *rpar;

#define ASCII_ARMAG	"!<arch>\n"
#define ASCII_SARMAG	8
#define ASCII_ARFMAG	"`\n"

/* ACK ar header. */

#define	ACK_ARMAG	0177545
#define ACK_AALMAG	0177454

/* Search an archive for a file and return that file's stat info. */
int archive_stat(char *name, struct stat *stp)
{
    int n, afd;
    int r = -1;
    char magic[8];
    char *file;
    static dev_t ardev;
    static ino_t arino = 0;
    static time_t armtime;

    if (!is_archive_ref(name)) {
	errno = EINVAL;
	return -1;
    }
    *lpar = 0;
    *rpar = 0;
    file = lpar + 1;

    if (stat(name, stp) < 0)
	goto bail_out;

    if (stp->st_ino != arino || stp->st_dev != ardev) {
	/* Either the first (and probably only) library, or a different
	 * library.
	 */
	arino = stp->st_ino;
	ardev = stp->st_dev;
	armtime = stp->st_mtime;
	deltab();

	if ((afd = open(name, O_RDONLY)) < 0)
	    goto bail_out;

	n = read(afd, magic, sizeof(magic));
	if (n == 8) {
	    if (strncmp(magic, ASCII_ARMAG, 8) == 0) {
		r = read_ascii_archive(afd);
	    } else if ((magic[0] & 0xFF) == ((ACK_AALMAG >> 0) & 0xFF) &&
		       (magic[1] & 0xFF) == ((ACK_AALMAG >> 8) & 0xFF)) {
		r = read_ack_archive(afd);
	    } else {
		errno = EINVAL;
		/* r = -1; */
	    }
	} else if (n == -1) {
	    /* r = -1 */ ;
	} else {
	    errno = EINVAL;
	}

	{
	    int e = errno;
	    close(afd);
	    errno = e;
	}
    } else {
	/* Library is cached. */
	r = 0;
    }

    if (r == 0) {
	/* Search the cache. */
	ulong mtime = convuzitime(&stp->st_mtime);
	r = searchtab(file, &mtime, 1);
	if (mtime > convuzitime(&armtime))
	    stp->st_mtime = armtime;
    }

  bail_out:
    /* Repair the name(file) thing. */
    *lpar = '(';
    *rpar = ')';
    return r;
}
