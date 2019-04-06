#include	<stdio.h>

extern long	lseek(), _fsize(), ftell();

fseek(f, offs, ptr)
register FILE *	f;
long		offs;
int		ptr;
{
	long	roffs;

	clreof(f);
	if(!f->_base)
		if(lseek(fileno(f), offs, ptr) == -1L)
			return -1;
		else
			return 0;
	if(f->_flag & _IOWRT)
		fflush(f);
	switch(ptr) {

	case 0:	/* relative to beginning of file */
		break;

	case 1:	/* relative to current postion */
		offs += ftell(f);
		break;

	case 2:	/* relative toend of file */
		offs += _fsize(fileno(f));
		break;

	default:
		return -1;
	}
	if((roffs = offs - ftell(f)) == 0)
		return 0;
	if(f->_flag & _IOREAD)
		if(roffs >= 0 && roffs <= f->_cnt) {
			f->_cnt -= roffs;
			f->_ptr += roffs;
			return 0;
		} else
			f->_cnt = 0;
	if(lseek(f->_file, offs, 0) == -1L)
		return -1;
	return 0;
}

long
ftell(f)
register FILE *	f;
{
	long	pos;

	pos = lseek(f->_file, 0L, 1);
	if(f->_cnt < 0)
		f->_cnt = 0;
	if(f->_base && f->_flag & _IOWRT)
		pos += BUFSIZ;
	return pos - f->_cnt;
}
