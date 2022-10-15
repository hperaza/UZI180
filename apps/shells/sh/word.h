/* -------- word.h -------- */
#ifndef WORD_H
#define	WORD_H	1

struct	wdblock {
	short	w_bsize;
	short	w_nword;
	/* bounds are arbitrary */
	char	*w_words[1];
};

struct wdblock *addword(char *wd, struct wdblock *wb);
struct wdblock *newword(int nw);
char **getwords(struct wdblock *wb);

#endif

