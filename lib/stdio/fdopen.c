/*
 *	fopen.c - stdio fopen 
 */

#include	<stdio.h>

extern int	open(char *, int), creat(char *, int);


FILE *
fdopen(fd, mode)
int fd; char * mode;
{
	register FILE *	f;
	uchar		c;

	for(f = _iob ; f != &_iob[_NFILE] ; f++)
		if(!(f->_flag & (_IOREAD|_IOWRT)))
			break;
	if(f == &_iob[_NFILE])
		return((FILE *)NULL);

	fclose(f);
	c = 0;
	f->_flag &= _IONBF;
	switch(*mode) {

	case 'w':
		c++;
	case 'a':
		c++;
	case 'r':
		if(mode[1] == 'b')
			f->_flag = _IOBINARY;
		break;

	}

	f->_file = fd;

	if(f->_file < 0)
		return (FILE *)NULL;
	if(!(f->_flag & (_IONBF|_IOMYBUF)))
		f->_base = _bufallo();
	if(f->_base == (char *)-1) {
		f->_base = (char *)0;
		/* close(f->_file); -- fd not ours */
		f->_flag = 0;
		return (FILE *)NULL;
	}
	f->_ptr = f->_base;
	f->_cnt = 0;
	if(c)
		f->_flag |= _IOWRT;
	else
		f->_flag |= _IOREAD;
	if(f->_base && c)
		f->_cnt = BUFSIZ;
	if(c == 1)
		fseek(f, 0L, 2);
	return f;
}
