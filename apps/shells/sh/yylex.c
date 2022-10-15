#include <setjmp.h>
#include <unix.h>

#include "yylex.h"
#include "sh.h"

/* defined in csyn.c: */

extern int startl, peeksym, iounit;
extern YYSTYPE yylval;

extern struct op *c_list(void);
extern void diag(int);

void yyerror(char *s);
int yylex(int cf);
static int dual(int c);
static int collect(int c, int c1);

static int nlseen;

int
yyparse()
{
	startl  = 1;
	peeksym = 0;
	yynerrs = 0;
	outtree = c_list();
	musthave('\n', 0);
	return(yynerrs!=0);
}

void
yyerror(s)
char *s;
{
	yynerrs++;
	if (talking && e.iop <= iostack) {
		multiline = 0;
		while (eofc() == 0 && yylex(0) != '\n')
			;
	}
	err(s);
	fail();
}

int
yylex(cf)
int cf;
{
	register int c, c1;
	int atstart;

	if ((c = peeksym) > 0) {
		peeksym = 0;
		if (c == '\n')
			startl = 1;
		return(c);
	}
	nlseen = 0;
	e.linep = line;
	atstart = startl;
	startl = 0;
	yylval.i = 0;

loop:
	while ((c = getc(0)) == ' ' || c == '\t')
		;
	switch (c) {
	default:
		if (any(c, "0123456789")) {
			unget(c1 = getc(0));
			if (c1 == '<' || c1 == '>') {
				iounit = c - '0';
				goto loop;
			}
			*e.linep++ = c;
			c = c1;
		}
		break;

	case '#':
		while ((c = getc(0)) != 0 && c != '\n')
			;
		unget(c);
		goto loop;

	case 0:
		return(c);

	case '$':
		*e.linep++ = c;
		if ((c = getc(0)) == '{') {
			if ((c = collect(c, '}')) != '\0')
				return(c);
			goto pack;
		}
		break;

	case '`':
	case '\'':
	case '"':
		if ((c = collect(c, c)) != '\0')
			return(c);
		goto pack;

	case '|':
	case '&':
	case ';':
		if ((c1 = dual(c)) != '\0') {
			startl = 1;
			return(c1);
		}
		startl = 1;
		return(c);
	case '^':
		startl = 1;
		return('|');
	case '>':
	case '<':
		diag(c);
		return(c);

	case '\n':
		nlseen++;
		gethere();
		startl = 1;
		if (multiline || cf & CONTIN) {
			if (talking && e.iop <= iostack)
				prs(cprompt->value);
			if (cf & CONTIN)
				goto loop;
		}
		return(c);

	case '(':
	case ')':
		startl = 1;
		return(c);
	}

	unget(c);

pack:
	while ((c = getc(0)) != 0 && !any(c, "`$ '\"\t;&<>()|^\n"))
		if (e.linep >= elinep)
			err("word too long");
		else
			*e.linep++ = c;
	unget(c);
	if(any(c, "\"'`$"))
		goto loop;
	*e.linep++ = '\0';
	if (atstart && (c = rlookup(line))!=0) {
		startl = 1;
		return(c);
	}
	yylval.cp = strsave(line, areanum);
	return(WORD);
}

static int
dual(c)
register c;
{
	char s[3];
	register char *cp = s;

	*cp++ = c;
	*cp++ = getc(0);
	*cp = 0;
	if ((c = rlookup(s)) == 0)
		unget(*--cp);
	return(c);
}

static int
collect(c, c1)
register c, c1;
{
	char s[2];

	*e.linep++ = c;
	while ((c = getc(c1)) != c1) {
		if (c == 0) {
			unget(c);
			s[0] = c1;
			s[1] = 0;
			prs("no closing "); yyerror(s);
			return(YYERRCODE);
		}
		if (talking && c == '\n' && e.iop <= iostack)
			prs(cprompt->value);
		*e.linep++ = c;
	}
	*e.linep++ = c;
	return(0);
}
