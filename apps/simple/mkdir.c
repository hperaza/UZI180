#include <unix.h>
#include <stdio.h>
#include <string.h>

unsigned short newmode;

int  mkdir(char *path, int mode);

int make_dir(name, f)
char *name;
int f;
{
    char *line;
    char iname[256];

    strcpy(iname, name);
    if (((line = rindex(iname, '/')) != NULL) && f) {
	while ((line > iname) && (*line == '/'))
	    --line;
	line[1] = 0;
	make_dir(iname, 1);
    }
    if (mkdir(name, newmode) && !f)
	return 1;
    else
	return 0;
}


int main(argc, argv)
int argc;
char **argv;
{
    int i, parent = 0, er = 0;

    if ((argv[1][0] == '-') && (argv[1][1] == 'p'))
	parent = 1;

    newmode = 0777 & ~umask(0);

    for (i = parent + 1; i < argc; i++) {
	if (argv[i][0] != '-') {
	    if (argv[i][strlen(argv[i]) - 1] == '/')
		argv[i][strlen(argv[i]) - 1] = '\0';

	    if (make_dir(argv[i], parent)) {
		fprintf(stderr, "mkdir: cannot create directory %s", argv[i]);
		perror("");
		er = 1;
	    }
	} else {
	    fprintf(stderr, "mkdir: usage error\n");
	    exit(1);
	}
    }
    exit(er);
}

int mkdir(char *path, int mode)
{
    char *p, dot[256], parent[256];
    int  _mknod(), _link();

    if (_mknod(path, 040000 | mode, 0) != 0) return errno;

    strcpy(dot, path);
    strcat(dot, "/.");

    if (_link(path, dot) != 0) return errno;

    strcpy(dot, path);
    strcat(dot, "/..");

    p = rindex(path, '/');
    if (p == NULL) {
        strcpy(parent, ".");
    } else {
        while (p > path && *p == '/') --p;
        strncpy(parent, path, p - path + 1);
        parent[p - path + 1] = 0;
    }

    if (_link(parent, dot) != 0) return errno;

    return 0;
}
