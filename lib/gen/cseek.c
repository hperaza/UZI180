#include "unix.h"
#include "stdio.h"


long
_ltell(fd)
uchar fd;
{
	long pos;
	uint16 blkno, offset;
	int _seek();
	
	blkno  = _seek(fd, 0, 4);
	offset = _seek(fd, 0, 1);
	
        pos = blkno * 512L + (long) offset;
        
	return pos;
}


long
lseek(fd, offs, whence)
uchar	fd, whence;
long	offs;
{
	uint16 blkno, offset;
	int _seek();

	offset = offs & 511L;
	blkno = offs >> 9L;

	/* caution: seek calls must be in this order */

	offset = _seek(fd, offset, whence);
	blkno  = _seek(fd, blkno, whence + 3);

	return (((long) blkno << 9L) + (long) offset);
}

long
_fsize(fd)
uchar   fd;
{
	struct stat stbuf;
        int _fstat();
        
	_fstat(fd, &stbuf);

	return (((long) stbuf.st_size.o_blkno << 9L) +
                 (long) stbuf.st_size.o_offset);
}
