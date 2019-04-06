#include <stdio.h>
#include <string.h>
#include <unix.h>

int lsmtab();
int add2mtab(char *, char *, char *, char *);

main(argc, argv)
int argc;
char *argv[];
{
    int _mount();

    if (argc == 1) {
        lsmtab();
        return 0;
    }

    if (argc != 3) {
        printf("usage: mount device path\n");
        return 1;
    }

    if (_mount(argv[1], argv[2], 0) == 0) {
        add2mtab(argv[1], argv[2], "uzi", "rw");
    } else {
        perror("mount");
        return 1;
    }
    return 0;
}

char tmp[256];

lsmtab()
{
    FILE *f;
    static char dev[20], mntpt[20], fstype[20], rwflag[20];
    
    f = fopen("/etc/mtab", "r");
    if (f) {
        while (fgets(tmp, 256, f)) {
            sscanf(tmp, "%s %s %s %s\n", dev, mntpt, fstype, rwflag);
            if (strcmp(fstype, "swap") == 0)
                printf("%s is swapspace\n", dev);
            else
                printf("%s mounted on %s read-%s\n", dev, mntpt,
                       (strcmp(rwflag, "ro") == 0) ? "only" : "write");
        }
        fclose(f);
    }
    
    return 0;
}

extern char *tmpnam(char *);

int rename(char *srcname, char *dstname)
{
    extern int _link(), _unlink();
    
    if (_link(srcname, dstname) < 0) {
        return -1;
    }

    if (_unlink(srcname) < 0) {
        _unlink(dstname);
        return -1;
    }

    return 0;
}


add2mtab(dev, mntpt, fstype, rwflag)
char *dev, *mntpt, *fstype, *rwflag;
{
    FILE *inpf, *outf;
    char *tmp_fname;
    extern int unlink();

    if ((tmp_fname = tmpnam(NULL)) == NULL) {
        perror("Error getting temporary file name");
        exit(1);
    }
    inpf = fopen("/etc/mtab", "r");
/*
    if (!inpf) {
        perror("Can't open /etc/mtab");
        exit(1);
    }
*/
    outf = fopen(tmp_fname, "w");
    if (!outf) {
        perror("Can't create temporary file");
        exit(1);
    }
    if (inpf) {
        while (fgets(tmp, 255, inpf)) {
            fprintf(outf, "%s", tmp);
        }
        fclose(inpf);
    }
    fprintf(outf, "%s %s %s %s\n", dev, mntpt, fstype, rwflag);
    fclose(outf);
    if (inpf && unlink("/etc/mtab") < 0) {
        perror("Can't delete old /etc/mtab file");
        exit(1);
    }
    if (rename(tmp_fname, "/etc/mtab") < 0) {
        perror("Error installing /etc/mtab");
        exit(1);
    }
    return 0;
}
