#include <stdlib.h>
#include "dirent.h"

#ifndef S_ISDIR
#define S_ISDIR(x)  (((x) & F_MASK) == F_DIR)
#endif

int stat(), open(), read(), close();

DIR *opendir(dname)
char *dname;
{
    int fd;
    DIR *p;
    struct stat st;

    if (stat(dname, &st) < 0)
	return 0;

    if (!S_ISDIR(st.st_mode)) {
	errno = ENOTDIR;
	return 0;
    }

    if ((fd = open(dname, O_RDONLY)) < 0)
	return 0;

    p = malloc(sizeof(DIR));
    if (p == 0) {
	close(fd);
	return 0;
    }

    p->dd_fd = fd;
    p->dd_loc = p->dd_size = 0;

    return p;
}

int closedir(dirp)
DIR *dirp;
{
    int fd;
    fd = dirp->dd_fd;
    free(dirp);
    return close(fd);
}

struct direct *readdir(dirp)
DIR *dirp;
{
    int cc;

    do {
        cc = read(dirp->dd_fd, &dirp->dd_buf, 16);
        if (cc < 16) return 0;
    } while (dirp->dd_buf.d_name[0] == '\0');

    return &dirp->dd_buf;
}
