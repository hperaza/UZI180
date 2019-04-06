#include <unix.h>
#include <stdio.h>
#include <string.h>

unsigned short newmode;

char *rindex(char *, char);
int  rmdir(char *);

int remove_dir(name, f)
char *name;
int f;
{
    int er, era = 2;
    char *line;

    while (((er = rmdir(name)) == 0)
	   && ((line = rindex(name, '/')) != NULL) && f) {
	while ((line > name) && (*line == '/')) --line;
	line[1] = 0;
	era = 0;
    }
    return (er && era);
}


int main(argc, argv)
int argc;
char **argv;
{
    int i, parent = 0, er = 0;

    if ((argv[1][0] == '-') && (argv[1][1] == 'p'))
	parent = 1;

    newmode = 0666 & ~umask(0);

    for (i = parent + 1; i < argc; i++) {
	if (argv[i][0] != '-') {
	    while (argv[i][strlen(argv[i]) - 1] == '/')
		argv[i][strlen(argv[i]) - 1] = '\0';
	    if (remove_dir(argv[i], parent)) {
	        fprintf(stderr, "rmdir: cannot remove directory %s\n", argv[i]);
		er = 1;
	    }
	} else {
	    fprintf(stderr, "rmdir: usage error\n");
	    exit(1);
	}
    }
    exit(er);
}

int rmdir(path)
char *path;
{
    int    fd;
    struct stat statbuf;
    char   newpath[100];
    struct direct dir;
    int    _stat(), _open(), _read(), strcmp(), _unlink();

    if (_stat(path, &statbuf) != 0) return errno;
    if ((statbuf.st_mode & F_DIR) == 0) return (errno = ENOTDIR);

    if ((fd = _open(path, 0)) < 0) return errno;

    while (_read(fd, (char *)&dir, sizeof(dir)) == sizeof(dir)) {
        if (dir.d_ino == 0) continue;
        if (!strcmp(dir.d_name, ".") || !strcmp(dir.d_name, ".."))
            continue;

        printf("rmdir: %s is not empty\n", path);
        _close(fd);
        return (errno = EACCES);  /* ENOTEMPTY */
    }
    _close(fd);

    strcpy(newpath, path);
    strcat(newpath, "/.");
    if (_unlink(newpath) != 0) return errno;

    strcat(newpath, ".");
    if (_unlink(newpath) != 0) return errno;

    if (_unlink(path) != 0) return errno;

    return 0;
}
