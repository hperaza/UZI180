#include <stdio.h>
#include <string.h>
#include <dirent.h>

extern int _getfsys(), stat();

typedef int16 ino_t;
typedef int16 dev_t;

char *devname(dev_t);
char *mntpoint(char *);

int main(argc, argv)
int argc; char *argv[];
{
    char *p;
    int  i, j, Total, Used, Free, Percent, iflag = 0, kflag = 0, fflag = 0;
    struct filesys fsys;

    for (i = 1; i < argc; ++i) {
	p = argv[i];
	if (p[0] == '-') {
	    for (++p; *p; ++p) {
		switch (*p) {
		case 'i': iflag = 1; break;
		case 'k': kflag = 1; break;
		case 'f': fflag = 1; break;
		default:
		    printf("usage: %s [-ikf]\n", argv[0]);
		    return 1;
		}
	    }
	} else {
	    break;
	}
    }

    printf("%-16s %6s %6s %6s %6s %s\n",
           "Filesystem",
            iflag ? "Inodes" : kflag ? "KBytes" : "Blocks",
            iflag ? "IUsed" : "Used",
            iflag ? "IFree" : "Free",
            iflag ? (fflag ? "%IFree" : "%IUsed") : (fflag ? "%Free" : "%Used"),
            "Mounted on");

    if (i < argc) {
	for (; i < argc; ++i) {
	    p = argv[i];
	}
    } else {
	for (j = 0; j < 8; ++j) {
	    if ((_getfsys(j, &fsys) == 0) && fsys.s_mounted) {
		Total = iflag ? 8 * (fsys.s_isize - 2) : fsys.s_fsize;
		Used  = iflag ? Total - fsys.s_tinode :
		                Total - fsys.s_isize - fsys.s_tfree;
		Free  = iflag ? fsys.s_tinode : fsys.s_tfree;

		if (!iflag && kflag) {
		    Total /= 2;
		    Used /= 2;
		    Free /= 2;
		}
		if (fflag) {
		  if ((Percent = Total / 100) != 0) Percent = Free / Percent;
		} else {
		  if ((Percent = Total / 100) != 0) Percent = Used / Percent;
		}
		p = devname(j);
		printf("%-16s %6u %6u %6u %5u%% %s\n",
		       p, Total, Used, Free, Percent,
		       fsys.s_mntpt ? mntpoint(p) : "/");
	    }
	}
    }

    return 0;
}

/* Search /dev until an entry with the specified device number is found */

#define DEV_PATH   "/dev"

extern int stat();

char *devname(devno)
dev_t devno;
{
    DIR *dp;
    struct direct *entry;
    struct stat fstat;
    static char namebuf[sizeof(DEV_PATH) + MAXNAMLEN + 2];

    if ((dp = opendir(DEV_PATH)) != (DIR *) NULL) {
	while ((entry = readdir(dp)) != (struct direct *) NULL) {
	    sprintf(namebuf, "%s/%s", DEV_PATH, entry->d_name);
	    if (stat(namebuf, &fstat) != 0)
		continue;
	    if ((fstat.st_mode & F_MASK) != F_BDEV)
		continue;
	    if (fstat.st_rdev != devno)
		continue;
	    return namebuf;
	}
    }

    sprintf(namebuf, "%d", devno);
    return namebuf;
}

/* Find the mount point in /etc/mtab for the specified device */

char *mntpoint(devname)
char *devname;
{
    FILE *f;
    static char tmp[256];
    static char dev[20], mntpt[20], fstype[20], rwflag[20];
    
    f = fopen("/etc/mtab", "r");
    if (f) {
        while (fgets(tmp, 256, f)) {
            sscanf(tmp, "%s %s %s %s\n", dev, mntpt, fstype, rwflag);
            if (strcmp(dev, devname) == 0) {
                fclose(f);
                return mntpt;
            }
        }
        fclose(f);
    }

    return "???";
}
