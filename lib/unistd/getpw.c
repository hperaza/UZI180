#include "unix.h"
#include <pwd.h>
#include <stdlib.h>
#include <string.h>

int _open(), _close(), _read();

static int passwd_fd = -1;

static int nread = 0;

static char *eptr, ebfr[256];
static char *pwdptr, pwdbfr[512];

char *pwd_fname = NULL;

/* close the password file */

void endpwent()
{
    if (passwd_fd < 0) return;
    _close(passwd_fd);
    passwd_fd = -1;
    nread = 0;
}

/* open the password file */

int setpwent()
{
    if (passwd_fd >= 0) _close(passwd_fd);
    if (pwd_fname == NULL) pwd_fname = "/etc/passwd";
    passwd_fd = _open(pwd_fname, O_RDONLY);
    if (passwd_fd < 0) return -1;
    return 0;
}

void setpwdfname(char *fname)
{
    endpwent();
    pwd_fname = fname;
}

/* read in the next line from the password file */

static int _pwdread()
{
    eptr = ebfr;
    do {
        if (nread == 0) {
            if ((nread = _read(passwd_fd, pwdbfr, 512)) <= 0) return 0;
            pwdptr = pwdbfr;
        }
        /* check for buffer overflow */
        if (eptr == ebfr + sizeof(ebfr)) return 0;
        --nread;
    } while ((*eptr++ = *pwdptr++) != '\n');
    eptr = ebfr;
    return 1;
}

/* return a pointer to the next field 
 * or NULL if the end of the field is reached.
 */

char *_pwdfield()
{
    char *p = eptr, *last;

    for (;;) {
        last = eptr;
        if (!*eptr) return NULL;
        if (*eptr == '\n') break;
        if (*eptr++ == ':') break;
    }
    *last = '\0';

    return p;
}

/* get the next password entry */

struct passwd *getpwent() {
    char *p;
    
    static struct passwd pwd;

    if (passwd_fd < 0) {
        if (setpwent() < 0) return NULL;
    }
    while (_pwdread()) {
        if ((pwd.pw_name = _pwdfield()) == 0) continue;
        if ((pwd.pw_passwd = _pwdfield()) == 0) continue;
        if ((p = _pwdfield()) == 0) continue;
        pwd.pw_uid = atoi(p);
        if ((p = _pwdfield()) == 0) continue;
        pwd.pw_gid = atoi(p);
        if ((pwd.pw_gecos = _pwdfield()) == 0) continue;
        if ((pwd.pw_dir = _pwdfield()) == 0) continue;
        if ((pwd.pw_shell = _pwdfield()) == 0) continue;
        if (*eptr == 0) return &pwd;
    }
    return NULL;
}

struct passwd *getpwuid(int uid)
{
    struct passwd *pwd;

    endpwent();
    while ((pwd = getpwent()) != NULL) {
        if (pwd->pw_uid == uid) break;
    }
    endpwent();
    return pwd;
}

struct passwd *getpwnam(char *name)
{
    struct passwd *pwd;

    endpwent();
    while ((pwd = getpwent()) != NULL) {
        if (strcmp(pwd->pw_name, name) == 0) break;
    }
    endpwent();
    return pwd;
}
