#include	<stdio.h>

extern	int	errno;
extern	char *	sys_err[];
extern	int	sys_ner;

static
ps(s)
char *	s;
{
	while(*s)
		putc(*s++, stderr);
}

perror(s)
char *	s;
{
	ps(s);
	putc(':', stderr);
	putc(' ', stderr);
	if(errno < sys_ner)
		ps(sys_err[errno]);
	else
		ps("Unknown error");
	putc('\n', stderr);
}
