#include <stdio.h>
#include <string.h>
#include <unix.h>

int rm_mtab(char *);
char *getdev(char *);

main(argc, argv)
int argc;
char *argv[];
{
    char *dev;
    int _umount();

    if (argc != 2) {
        printf("usage: umount device\n");
        return 1;
    }

    dev = getdev(argv[1]);
    if (!dev) dev = argv[1];

    if (_umount(dev) == 0) {
        rm_mtab(dev);
    } else {
        perror("umount");
        return 1;
    }
    return 0;
}

char tmp[256];

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


char *getdev(arg)
char *arg;
{
    FILE *f;
    static char dev[20], mntpt[20], fstype[20], rwflag[20];
    
    f = fopen("/etc/mtab", "r");
    if (f) {
        while (fgets(tmp, 256, f)) {
            sscanf(tmp, "%s %s %s %s\n", dev, mntpt, fstype, rwflag);
            if ((strcmp(dev, arg) == 0) || (strcmp(mntpt, arg) == 0)) {
                fclose(f);
                return dev;
            }
        }
        fclose(f);
    }

    return NULL;
}


rm_mtab(devname)
char *devname;
{
    FILE *inpf, *outf;
    char *tmp_fname;
    static char dev[20], mntpt[20], fstype[20], rwflag[20];
    extern int unlink();

    if ((tmp_fname = tmpnam(NULL)) == NULL) {
        perror("Error getting temporary file name");
        exit(1);
    }
    inpf = fopen("/etc/mtab", "r");
    if (!inpf) {
        perror("Can't open /etc/mtab");
        exit(1);
    }
    outf = fopen(tmp_fname, "w");
    if (!outf) {
        perror("Can't create temporary file");
        exit(1);
    }
    while (fgets(tmp, 255, inpf)) {
        sscanf(tmp, "%s %s %s %s\n", dev, mntpt, fstype, rwflag);
        if (strcmp(dev, devname) == 0) {
            continue;
        } else {
            fprintf(outf, "%s", tmp);
        }
    }
    fclose(inpf);
    fclose(outf);
    if (unlink("/etc/mtab") < 0) {
        perror("Can't delete old /etc/mtab file");
        exit(1);
    }
    if (rename(tmp_fname, "/etc/mtab") < 0) {
        perror("Error installing /etc/mtab");
        exit(1);
    }
    return 0;
}
