/* find - look for files satisfying a predicate       Author: E. Baalbergen */
/* Original author: Erik Baalbergen; POSIX compliant version: Bert Laverman */
/* UZI180 port by Hector Peraza */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#if 0
#include <time.h>
#endif
#include <pwd.h>
#include <grp.h>

#include "find.h"

/*########################### PARSER ###################################*/
/* Grammar:
 * expr        : primary | primary OR expr;
 * primary     : secondary | secondary AND primary | secondary primary;
 * secondary   : NOT secondary | LPAR expr RPAR | simple;
 * simple      : -OP args...
 */

/* Isnumber checks correct number syntax. A sign is allowed, but the '+'
 * only if the number is to be in decimal.
 */
int isnumber(char *str, int base, int sign)
{
    if (sign && ((*str == '-') || ((base == 8) && (*str == '+'))))
	str++;
    while ((*str >= '0') && (*str < ('0' + base)))
	str++;
    return (*str == '\0' ? 1 : 0);
}

/* Convert a string to an integer, storing sign info in *ps. */
void number(char *str, int base, long *pl, int *ps)
{
    int up = '0' + base - 1;
    long val = 0;

    *ps = ((*str == '-' || *str == '+') ? ((*str++ == '-') ? -1 : 1) : 0);
    while (*str >= '0' && *str <= up)
	val = base * val + *str++ - '0';
    if (*str)
	fatal("syntax error: illegal numeric value", "");
    *pl = val;
}

struct node *expr(int t)
{
    struct node *nd, *p, *nd2;

    nd = primary(t);
    if ((t = lex(*++ipp)) == OP_OR) {
	nd2 = expr(lex(*++ipp));
	p = newnode(OP_OR);
	p->n_info.n_opnd.n_left = nd;
	p->n_info.n_opnd.n_right = nd2;
	return p;
    }
    ipp--;
    return nd;
}

struct node *primary(int t)
{
    struct node *nd, *p, *nd2;

    nd = secondary(t);
    if ((t = lex(*++ipp)) != OP_AND) {
	ipp--;
	if (t == EOI || t == RPAR || t == OP_OR)
	    return nd;
    }
    nd2 = primary(lex(*++ipp));
    p = newnode(OP_AND);
    p->n_info.n_opnd.n_left = nd;
    p->n_info.n_opnd.n_right = nd2;
    return p;
}

struct node *secondary(int t)
{
    struct node *n, *p;

    if (t == LPAR) {
	n = expr(lex(*++ipp));
	if (lex(*++ipp) != RPAR)
	    fatal("syntax error, ) expected", "");
	return n;
    }
    if (t == NOT) {
	n = secondary(lex(*++ipp));
	p = newnode(NOT);
	p->n_info.n_opnd.n_left = n;
	return p;
    }
    return simple(t);
}

void checkarg(char *arg)
{
    if (arg == 0)
	fatal("syntax error, argument expected", "");
}
