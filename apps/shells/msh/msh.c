/* mini-shell for UZI180 - (C) 2000-2003, Hector Peraza  */
/* Should compile and run under Linux as well (no, it is */
/* not intended to substitute bash, it just makes easier */
/* the development and testing).                         */

/* See the TODO comments through the text */

#ifdef UZI
#include <unix.h>
#else
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>

#include "filmatch.h"

#ifndef UZI
#define direct dirent
#endif

#ifdef UZI
extern int open();
extern int creat(char *filename, int mode);
extern int pipe(int fd[2]);
extern int dup(int fd);
extern int read(int fd, void *buffer, int n);
extern int close(int fd);
extern int execve();
extern int fork();
extern int wait(int pid);
extern int chdir(char *path);
extern int getuid();
extern int execvp(char *file, char *argv[]);
#endif

extern char **environ;

#ifndef STDIN_FILENO
#define STDIN_FILENO   0
#define STDOUT_FILENO  1
#define STDERR_FILENO  2
#endif

#ifdef UZI
#define O_TRUNC  0x10
#define O_APPEND 0x20
#define O_CREAT  0x40

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

char *progname, ***args = NULL, *fstdin, *fstdout, *fstderr;
char c, buf[128], eline[128], *pb;
int  bg, nargs, cmdc, pd[2], d, ofilemode, efilemode;

DIR *dirp;
struct direct *dp;
char *pattern, *dname;

void prompt();
int  parse();
void execute();
void skipblanks();
char *getword();
char *getquoted(char quote);
#ifdef UZI
char *strdup(char *str);
#endif
char *stradd(char *str, char *nw);
char *strnadd(char *str, char *nw, int len);
int  isdelim();

int  doumask(char *);
int  eopen(char *, int, int);


/* some utility functions... */

char *basename(char *name)
{
    char *base;

    base = rindex(name, '/');
    return base ? base + 1 : name;
}

char *dirname(char *name)
{
    char *p;
    static char dir[256];

    p = rindex(name, '/');
    if (p == NULL) {
        strcpy(dir, ".");
    } else {
        while (p > name && *p == '/') --p;
        strncpy(dir, name, p - name + 1);
        dir[p - name + 1] = 0;
    }

    return dir;
}

/* display the command prompt as "cwd $" or "cwd #" */

void prompt()
{
    char dir[100], cprompt;

    getcwd(dir, 100);
    cprompt = (getuid() == 0) ? '#' : '$';
    printf("%s %c ", dir, cprompt);
}

int main(int argc, char *argv[])
{
    char *home;
    FILE *inpf;
    int  login_shell = 0, temp_shell = 0;

    progname = argv[0];
    inpf = stdin;

    if (progname[0] == '-') {
        login_shell = 1;
        ++progname;
    }

    if (argc > 1) {
        if (strcmp(argv[1], "-l") == 0) {
            login_shell = 1;
        } else if (strcmp(argv[1], "-c") == 0) {
            temp_shell = 1;
            if (argc < 3) exit(1);
        }
    }

    /* setup signal handlers */

    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);

    /* if invoked a login shell, cd to $HOME and execute startup file */

    if (login_shell) {
	home = getenv("HOME");
        if (!home || chdir(home)) {
	    fprintf(stderr, "%s: no home directory, using \"/\"\n", progname);
            putenv("HOME=/");
            chdir(home = "/");
        }
        inpf = fopen(".mshrc", "r");
        if (!inpf) inpf = stdin;
    }

    /* the main loop: get command line, parse it, execute it. */

    for (;;) {

	fstdin  = NULL;
	fstdout = NULL;
	fstderr = NULL;

        pattern = NULL;
        dirp = NULL;

        args = realloc(args, sizeof(char *));
        args[0] = NULL;

	if (temp_shell) {
	    pb = argv[2];
	} else {
	    if (inpf == stdin) prompt();
            fgets(buf, 127, inpf);
            if (feof(inpf)) {
               if (inpf == stdin) {
                   printf("\n");
                   return 0;
               }
               fclose(inpf);
               inpf = stdin;
               continue;
           }
	   buf[strlen(buf) - 1] = '\0';   /* strip newline */
	   pb = buf;
        }

	if (parse()) {
	    execute();
	} else {
	    --pb;
	    eline[0] = *pb++;
	    eline[1] = *pb;
	    eline[2] = '\0';
	    fprintf(stderr, "%s: syntax error near \"%s\"\n", progname, eline);
	}

	if (fstdin)  free(fstdin);
	if (fstdout) free(fstdout);
	if (fstderr) free(fstderr);

        if (pattern) free(pattern);
        if (dirp)    closedir(dirp);

	if (temp_shell) exit(0);  /* we should return the exit code of the called program instead */
    }
}

int parse()
{
    int filemode;
    bg = cmdc = nargs = ofilemode = efilemode = 0;
    skipblanks();
    while (dirp || *pb) {
	if (!dirp && *pb && isdelim()) {
	    c = *pb++;
	    switch (c) {
	    case '<':
	        if (fstdin) return 0;
		fstdin = getword();
		break;

	    case '>':
		filemode = O_CREAT | O_TRUNC | O_WRONLY;
		if (*pb == '>') {
		    ++pb;
		    filemode = O_CREAT | O_APPEND | O_WRONLY;
		}
	        if (*pb == '&') {
	            ++pb;
	            efilemode = filemode;
	            if (fstderr) return 0;
	            fstderr = getword();
	        } else {
	            ofilemode = filemode;
	            if (fstdout) return 0;
		    fstdout = getword();
		}
		break;

	    case '|':
		if (args[cmdc]) {
		    cmdc++;
		    nargs = 0;
	    	}
		break;

	    case '&':
		bg = 1;
		break;
	    }
	} else {
	    if (!args[cmdc]) {
		args = realloc(args, (cmdc + 2) * sizeof(char *));
		args[cmdc] = realloc(args[cmdc], sizeof(char *));
		args[cmdc][0] = NULL;
		args[cmdc + 1] = NULL;
	    }
	    args[cmdc] = realloc(args[cmdc], (nargs + 2) * sizeof(char *));
	    args[cmdc][nargs++] = getword();
	    args[cmdc][nargs] = NULL;
	}
	skipblanks();
    }

    return 1;
}


void execute()
{
    int id;

    for (cmdc = 0; args[cmdc]; ) {

	if (!cmdc) d = dup(STDIN_FILENO);

	if (args[cmdc + 1]) {
	    if (pipe(pd) < 0) {
	        fprintf(stderr, "%s: ", progname);
		perror("pipe");
		exit(-1);
	    }
	}

	if (!strcmp(args[cmdc][0], "exit")) {
	    id = 0;
	    if (args[cmdc][1]) id = atoi(args[cmdc][1]);
	    exit(id);

	} else if (!strcmp(args[cmdc][0], "cd")) {
	    if (args[cmdc][1]) {
		if (chdir(args[cmdc][1]) < 0) perror("cd");
	    } else {
		chdir(getenv("HOME"));
	    }

	} else if (!strcmp(args[cmdc][0], "fg")) {
	    /* TODO: redirect stdin to the specified process
	       and loop while(wait(NULL) != spec proc id) */

	} else if (!strcmp(args[cmdc][0], "sync")) {
	    sync();

	} else if ((pb = strchr(args[cmdc][0], '=')) != NULL) {
	    if (*(pb+1) == '\0') *pb = '\0';
	    putenv(args[cmdc][0]);

	} else if (!strcmp(args[cmdc][0], "umask")) {
	    doumask(args[cmdc][1]);

	} else {
	    id = fork();
	    if (id < 0) {
	    
		fprintf(stderr, "%s: ", progname);
		perror(args[cmdc][0]);
		
	    } else if (id == 0) {

		if (fstdin && !cmdc) {
		    /* redirect stdin */
		    close(STDIN_FILENO);
		    if (eopen(fstdin, O_RDONLY, 0666) < 0) {
			perror(fstdin);
			exit(-1);
		    }
		} else {
		    close(STDIN_FILENO);
		    dup(d);
		    close(d);
		}

		if (fstdout) {
		    /* redirect stdout */
		    close(STDOUT_FILENO);
		    if (eopen(fstdout, ofilemode, 0666) < 0) {
			perror(fstdout);
			exit(-1);
		    }
		}

		if (fstderr) {
		    /* redirect stderr */
		    close(STDERR_FILENO);
		    if (eopen(fstderr, efilemode, 0666) < 0) {
			perror(fstderr);
			exit(-1);
		    }
		}

		if (args[cmdc + 1]) {
		    close(STDOUT_FILENO);
		    dup(pd[1]);
		    close(pd[1]);
		    close(pd[0]);
		}

		if (!bg) {
		    signal(SIGINT, SIG_DFL);
		    signal(SIGQUIT, SIG_DFL);
		}

		/* TODO: if started on the background, we
		   should close or redirect to another place
		   the process' stdin (we should be able to
		   restore it in the 'fg' command) (we shouldn't
		   close it otherwise the program could die
		   because of EOF) */

		if (execvp(args[cmdc][0], args[cmdc]) < 0) {
		    fprintf(stderr, "%s: ", progname);
		    perror(args[cmdc][0]);
		    exit(-1);
                }
	    }

	    if (bg)
		printf("[%d]\n", id);

	    if (!args[cmdc + 1] && !bg)
		while (wait(NULL) != id);
	}

	for (id = 0; args[cmdc][id]; ++id) free(args[cmdc][id]);
	free(args[cmdc]);

	close(d);

	if (args[++cmdc]) {
	    d = dup(pd[0]);
	    close(pd[0]);
	    close(pd[1]);
	}
    }
}

#ifdef UZI
int execvp(char *file, char *argv[])
{
    int  i, asis;
    char *path, *sp, *tp;
    
    path = getenv("PATH");  /* get path string */
    if (!path) path = ":/bin:/usr/bin";
    eline[0] = '\0';
    sp = strchr(file, '/') ? "" : path;
    asis = (*sp == '\0');
    while (asis || *sp != '\0') {
	asis = 0;
	tp = eline;
	for ( ; *sp != '\0'; tp++)
	    if ((*tp = *sp++) == ':') {
		asis = (*sp == '\0');
		break;
	    }
	if (tp != eline)
	    *tp++ = '/';
	for (i = 0; (*tp++ = args[cmdc][0][i++]) != '\0'; ) ;
	execve(eline, argv, environ);
    }

    return -1;
}
#endif

int isdelim()
{
    return *pb == ' ' || *pb == '\t' ||
           *pb == '<' || *pb == '>' ||
           *pb == '|' || *pb == '&' ||
           !*pb;
}

void skipblanks()
{
    while (*pb == ' ' || *pb == '\t') ++pb;
}

char *getword()
{
    char *s, *s1, *s2, *str, quote;
    char *envDupExpand(char *, int);

    if (dirp) {
	while ((dp = readdir(dirp)) != NULL) {
	    if ((dp->d_name[0] == '.') && (pattern[0] != '.')) continue;
            if (match(pattern, dp->d_name)) {
        	if (strcmp(dname, ".") == 0) return strdup(dp->d_name);
        	str = strdup(dname);
        	if (str[strlen(str)-1] != '/') str = stradd(str, "/");
        	str = stradd(str, dp->d_name);
        	return str;
            }
	}
	closedir(dirp);
	dirp = NULL;
	free(pattern);
	pattern = NULL;
    }

    skipblanks();
    if (isdelim()) return NULL;
    quote = 0;
    s = pb;
    str = NULL;
    while (!isdelim()) {
      if (*pb == '"' || *pb == '\'') {

        s1 = strnadd(NULL, s, pb - s);
        s2 = envDupExpand(s1, 0);
        free(s1);
        str = stradd(str, s2);
        free(s2);

        quote = *pb++;
        s1 = getquoted(quote);
        if (quote == '"') {
          s2 = envDupExpand(s1, 0);
          free(s1);
          s1 = s2;
        }
        str = stradd(str, s1);
        free(s1);
        s = pb;

      } else {
        ++pb;
      }
    }
    s1 = strnadd(NULL, s, pb - s);
    if (strlen(s1) > 0) quote = 0;
    s2 = envDupExpand(s1, 0);
    free(s1);
    str = stradd(str, s2);
    free(s2);

    if (!quote && !strchr(str, '=') && is_pattern(str)) {
	dname = dirname(str);
	dirp = opendir(dname);
	if (dirp) {
	    pattern = strdup(basename(str));
	    free(str);
	    return getword();
	}
    }

    return str;
}

char *getquoted(char quote)
{
    char *s, *str;

    s = pb;
    while (*pb && *pb != quote) ++pb;
    str = (char *) malloc(pb - s + 1);
    strncpy(str, s, pb - s);
    str[pb - s] = '\0';
    ++pb;

    return str;
}

#ifdef UZI
char *strdup(char *str)
{
    char *s;
    s = malloc(strlen(str) + 1);
    strcpy(s, str);
    return s;
}
#endif

char *stradd(char *str, char *nw)
{
    char *s;
    s = realloc(str, (str ? strlen(str) : 0) + strlen(nw) + 1);
    if (str) strcat(s, nw); else strcpy(s, nw);
    return s;
}

char *strnadd(char *str, char *nw, int len)
{
    char *s;
    int  len1;
    
    len1 = (str ? strlen(str) : 0) + len;
    s = realloc(str, len1 + 1);
    if (str) strncat(s, nw, len); else strncpy(s, nw, len);
    s[len1] = '\0';

    return s;
}

int doumask(char *arg)
{
    int i, umask();

    if (arg == NULL) {
	i = umask(0);
	umask(i);
	printf("%03o\n", i);
    } else {
	i = 0;
	while (*arg >= '0' && *arg <= '7')
	    i = (i << 3) + *arg++ - '0';
	if (*arg || (i & ~0777))
	    fprintf(stderr, "umask: bad mask value\n");
	else
	    umask(i);
    }
}

int eopen(char *name, int flags, int mode) {
#ifdef UZI
    int fd, stat();
    struct stat stbuf;
    
    if (flags & O_TRUNC) {
        unlink(name);
        return creat(name, mode);
    }

    if ((flags & O_CREAT) && stat(name, &stbuf))
	return creat(name, mode);
    else
	fd = open(name, flags & (O_RDONLY | O_WRONLY | O_RDWR));
    
    if ((fd >= 0) && (flags & O_APPEND))
        lseek(fd, 0L, SEEK_END);

    return fd;
#else
    return open(name, flags, mode);
#endif
}
