#include <dirent.h>
#include <string.h>

typedef int16 ino_t;
typedef int16 dev_t;

/*
 * These functions find the absolute path to the current working directory.
 *
 * They don't use malloc or large amounts of stack space.
 */

static char *recurser();	/* Routine to go up tree */
static char *search_dir();	/* Routine to find the step back down */
static char *path_buf;
static int path_size;

static dev_t root_dev;
static ino_t root_ino;

static struct stat st;

int stat();

char *getcwd(buf, size)
char *buf;
int size;
{
    path_buf = buf;
    path_size = size;

    if (size < 3) {
	errno = ERANGE;
	return 0;
    }
    strcpy(path_buf, ".");

    if (stat("/", &st) < 0)
	return 0;

    root_dev = st.st_dev;
    root_ino = st.st_ino;

    return recurser();
}

static char *recurser()
{
    dev_t this_dev;
    ino_t this_ino;

    if (stat(path_buf, &st) < 0)
	return 0;

    this_dev = st.st_dev;
    this_ino = st.st_ino;

    if (this_dev == root_dev && this_ino == root_ino) {
	strcpy(path_buf, "/");
	return path_buf;
    }
    if (strlen(path_buf) + 4 > path_size) {
	errno = ERANGE;
	return 0;
    }
    strcat(path_buf, "/..");
    if (recurser() == 0)
	return 0;

    return search_dir(this_dev, this_ino);
}

static char *search_dir(this_dev, this_ino)
dev_t this_dev;
ino_t this_ino;
{
    DIR *dp;
    struct direct *d;
    char *ptr;
    int slen, slow_search;

    slow_search = 0;

    if (stat(path_buf, &st) < 0)
	return 0;

    if (this_dev != st.st_dev)
	slow_search = 1;

    slen = strlen(path_buf);
    ptr = path_buf + slen - 1;
    if (*ptr != '/') {
	if (slen + 2 > path_size) {
	    errno = ERANGE;
	    return 0;
	}
	strcpy(++ptr, "/");
	slen++;
    }
    slen++;

    dp = opendir(path_buf);
    if (dp == 0)
	return 0;

    while ((d = readdir(dp)) != 0) {
	if (slow_search || this_ino == d->d_ino) {
	    if (slen + strlen(d->d_name) > path_size) {
		errno = ERANGE;
		return 0;
	    }
	    strcpy(ptr + 1, d->d_name);
	    if (stat(path_buf, &st) < 0)
		continue;
	    if (st.st_ino == this_ino && st.st_dev == this_dev) {
		closedir(dp);
		return path_buf;
	    }
	}
    }

    closedir(dp);
    errno = ENOENT;
    return 0;
}
