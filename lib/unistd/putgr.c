#include "unix.h"
#include <grp.h>
#include <stdio.h>

int putgrent(struct group *grp, FILE *f)
{
    char **p;

    if (fprintf(f, "%s:%s:%d:\n",
                   grp->gr_name,
                   grp->gr_passwd,
                   grp->gr_gid) == 0) return -1;
    p = grp->gr_mem;
    while (p) {
       if (fprintf(f, "%s", p) == 0) return -1;
       if (++p) fputc(',', f);
    }
               
    if (ferror(f)) return -1;
    
    return 0;
}
