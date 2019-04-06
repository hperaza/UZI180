#include <stdio.h>
#include <unix.h>

main(argc, argv)
int argc;
char *argv[];
{
    int do_mknod();

    if (argc != 4) {
        printf("usage: mknod path modes devs\n");
        return 1;
    }

    return do_mknod(argv[1], argv[2], argv[3]);
}

do_mknod(path, modes, devs)
char *path;
char *modes;
char *devs;
{
    int mode;
    int dev;
    int _mknod();

    mode = -1;
    sscanf(modes, "%o", &mode);
    if (mode == -1) {
        printf("mknod: bad mode\n");
        return (-1);
    }
    
    if ((mode & F_MASK) != F_BDEV && (mode & F_MASK) != F_CDEV) {
        printf("mknod: mode is not device\n");
        return (-1);
    }

    dev = -1;
    sscanf(devs, "%d", &dev);
    if (dev == -1) {
        printf("mknod: bad device\n");
        return (-1);
    }

    if (_mknod(path, mode, dev) != 0) {
        perror("_mknod");
        return (-1);
    }

    return(0);
}
