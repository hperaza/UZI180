#include "unix.h"
#include <grp.h>
#include <stdlib.h>
#include <string.h>

int _open(), _close(), _read();

static int group_fd = -1;

static int nread = 0;

static char *eptr, ebfr[256];
static char *grpptr, grpbfr[512];
static char *gmem[17];

char *grp_fname = NULL;

/* close the group file */

void endgrent()
{
    if (group_fd < 0) return;
    _close(group_fd);
    group_fd = -1;
    nread = 0;
}

/* open the group file */

int setgrent()
{
    if (group_fd >= 0) _close(group_fd);
    if (grp_fname == NULL) grp_fname = "/etc/group";
    group_fd = _open(grp_fname, O_RDONLY);
    if (group_fd < 0) return -1;
    return 0;
}

void setgrpfname(char *fname)
{
    endgrent();
    grp_fname = fname;
}

/* read in the next line from the group file */

static int _grpread()
{
    eptr = ebfr;
    do {
        if (nread == 0) {
            if ((nread = _read(group_fd, grpbfr, 512)) == 0) return 0;
            grpptr = grpbfr;
        }
        /* check for buffer overflow */
        if (eptr == ebfr + sizeof(ebfr)) return 0;
        --nread;
    } while ((*eptr++ = *grpptr++) != '\n');
    eptr = ebfr;
    return 1;
}

/* return a pointer to the next field terminated by the specified
 * char, or NULL if the end of the field is reached.
 */

char *_grpfield(char delim)
{
    char *p = eptr, *last;

    for (;;) {
        last = eptr;
        if (!*eptr) return NULL;
        if (*eptr == '\n') break;
        if (*eptr++ == delim) break;
    }
    *last = '\0';

    return p;
}

/* get the next group entry */

struct group *getgrent() {
    int i;
    char *p;
    
    static struct group grp;

    if (group_fd < 0) {
        if (setgrent() < 0) return NULL;
    }
    while (_grpread()) {
        if ((grp.gr_name = _grpfield(':')) == 0) continue;
        if ((grp.gr_passwd = _grpfield(':')) == 0) continue;
        if ((p = _grpfield(':')) == 0) continue;
        grp.gr_gid = atoi(p);
        for (i = 0; *eptr && (i < 16); ++i) {
            if ((gmem[i] = _grpfield(',')) == 0) break;
        }
        gmem[i] = 0;
        grp.gr_mem = &gmem;
        if (*eptr == 0) return &grp;
    }
    return NULL;
}

struct group *getgrgid(int gid)
{
    struct group *grp;

    endgrent();
    while ((grp = getgrent()) != NULL) {
        if (grp->gr_gid == gid) break;
    }
    endgrent();
    return grp;
}

struct group *getgrnam(char *name)
{
    struct group *grp;

    endgrent();
    while ((grp = getgrent()) != NULL) {
        if (strcmp(grp->gr_name, name) == 0) break;
    }
    endgrent();
    return grp;
}
