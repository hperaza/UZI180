/*****************************************************************
 *                                                               *
 *  Linux utility to analyze Hi-Tech C library map files.        *
 *  Sorts library modules so symbols could be resolved after     *
 *  a single scan of the library, and proposes the new library   *
 *  module order.                                                *
 *                                                               *
 *  (C) 2001-2022, Hector Peraza.                                *
 *                                                               *
 *****************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

struct _symbol {
  char *name;     /* symbol name */
  int  type;      /* symbol type ('D'ef/'U'ndef) */
};

struct _modtab {
  char *name;                  /* module name */
  struct _symbol symbols[100]; /* symbol table for module (max 100 symbols) */
  int  nsym;                   /* number of symbols in symbol table */
};

struct _modtab modtab[10000];  /* wow... 8 Mbytes */

char *strpool[10000];
int  nstr = 0;

char *addstr(char *str);
int  cmpmod(const void *a, const void *b);

int main(char argc, char *argv[]) {
  FILE *f;
  char *p, line[256], module[20], symbol[20];
  int  nmod, nsym, type, i, k;
  
  if (argc != 2) {
    fprintf(stderr, "usage: %s mapfile\n", argv[0]);
    return 1;
  }
  
  f = fopen(argv[1], "r");
  if (!f) {
    fprintf(stderr, "File not found: %s\n", argv[1]);
    return 1;
  }

  /*printf("Reading %s...\n", argv[1]);*/

  nmod = nsym = 0;
  k = -1;
  while (!feof(f)) {
    fgets(line, 256, f);
    if (line[0] == '\n') continue;
    p = line;
    if (*p == 26) break;

    if (!isblank(*p)) {
      /* module name */
      i = 0;
      while (!isspace(*p)) module[i++] = *p++;
      module[i] = '\0';
      k = nmod++;
      /* printf("%s:\n", module); */
      modtab[k].name = addstr(module);
      modtab[k].nsym = 0;
    }

    if (k < 0) {
      fprintf(stderr, "File format error\n");
      return 1;
    }

    while (1) {
      while (isblank(*p)) ++p;
      if (*p == '\n' || *p == '\r' || *p == '\0') break;
      type = (int) *p++;
      while (isblank(*p)) ++p;
      i = 0;
      while (!isspace(*p)) symbol[i++] = *p++;
      symbol[i] = '\0';

      nsym = modtab[k].nsym;
      if (nsym == 100) {
        fprintf(stderr, "Symbol table overflow for module %s\n", modtab[k].name);
        return 1;
      }
   
      modtab[k].symbols[nsym].name = addstr(symbol);
      modtab[k].symbols[nsym].type = type;
      modtab[k].nsym++;
    }
  }

  /*printf("Sorting...\n");*/

  qsort(modtab, nmod, sizeof(modtab[0]), cmpmod);

  printf("Proposed module order\n");
  printf("---------------------\n");

  for (k = 0; k < nmod; ++k) {
    printf("%s\n", modtab[k].name);
  }

  printf("\n\n");
  printf("Proposed module map\n");
  printf("-------------------\n");
  
  for (k = 0; k < nmod; ++k) {
    printf("%-16s", modtab[k].name);
    nsym = 0;
    for (i = 0; i < modtab[k].nsym; ++i) {
      if (nsym++ == 4) {
        printf("\n\t\t");
        nsym = 0;
      }
      printf("%c %-14s", modtab[k].symbols[i].type, modtab[k].symbols[i].name);
    }
    printf("\n");
  }

  fclose(f);

  return 0;
}

int cmpmod(const void *a, const void *b) {
  struct _modtab *ma, *mb;
  int i, j;
  
  ma = (struct _modtab *) a;
  mb = (struct _modtab *) b;
  
  for (i = 0; i < ma->nsym; ++i) {
    if (ma->symbols[i].type == 'U') {
      for (j = 0; j < mb->nsym; ++j) {
#if 0
        if ((mb->symbols[j].type == 'D') &&
           (strcmp(ma->symbols[i].name, mb->symbols[j].name) == 0)) return -1;
#else
        if ((mb->symbols[j].type == 'D') &&
            (ma->symbols[i].name == mb->symbols[j].name)) return -1;
#endif
      }
    }
  }

  for (i = 0; i < mb->nsym; ++i) {
    if (mb->symbols[i].type == 'U') {
      for (j = 0; j < ma->nsym; ++j) {
#if 0
        if ((ma->symbols[j].type == 'D') &&
           (strcmp(ma->symbols[j].name, mb->symbols[i].name) == 0)) return 1;
#else
        if ((ma->symbols[j].type == 'D') &&
            (ma->symbols[j].name == mb->symbols[i].name)) return 1;
#endif
      }
    }
  }
  
  return strcmp(ma->name, mb->name);
}

char *addstr(char *str) {
  int  i;
  char *s;
  
  for (i = 0; i < nstr; ++i)
    if (strcmp(strpool[i], str) == 0) return strpool[i];
    
  s = strdup(str);
  if (!s) {
    fprintf(stderr, "Out of memory\n");
    exit(1);
  }

  strpool[nstr++] = s;
  return s;
}
