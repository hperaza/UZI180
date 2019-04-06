#include "unix.h"
#include <pwd.h>
#include <stdio.h>

int putpwent(struct passwd *pwd, FILE *f)
{
    if (fprintf(f, "%s:%s:%d:%d:%s:%s:%s\n",
                   pwd->pw_name,
                   pwd->pw_passwd,
                   pwd->pw_uid,
                   pwd->pw_gid,
                   pwd->pw_gecos,
                   pwd->pw_dir,
                   pwd->pw_shell) == 0) return -1;
               
    if (ferror(f)) return -1;
    
    return 0;
}
