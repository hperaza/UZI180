/*****************************************************************
 *                                                               *
 *  Linux utility to analyze Hi-Tech C library map files.        *
 *  Reports undefined symbols, and symbols which are defined     *
 *  before they are references and that therefore will remain    *
 *  undefined after a single scan of the library.                *
 *                                                               *
 *  (C) 2001-2022, Hector Peraza.                                *
 *                                                               *
 *****************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <string.h>

struct _symtab {
  char sym[20], refmod[20]; /* symbol name and module referencing it */
  int type, ref;            /* symbol type and last ref ('D'ef/'U'ndef) */
};

struct _symtab symtab[10000];

int main(char argc, char *argv[]) {
  FILE *f;
  char *p, line[256], module[20], symbol[20];
  int nmod, nsym, nundef, i, k, total, type;
  
  if (argc != 2) {
    fprintf(stderr, "usage: %s mapfile\n", argv[0]);
    return 1;
  }
  
  f = fopen(argv[1], "r");
  if (!f) {
    fprintf(stderr, "File not found: %s\n", argv[1]);
    return 1;
  }

  nmod = nsym = total = 0;
  while (!feof(f)) {
    fgets(line, 256, f);
    p = line;
    if (*p == 26) break;
    if (!isblank(*p)) {
      /* module name */
      i = 0;
      while (!isspace(*p)) module[i++] = *p++;
      module[i] = '\0';
      ++nmod;
      /* printf("%s:\n", module); */
    }

    while (1) {
      while (isblank(*p)) ++p;
      if (*p == '\n' || *p == '\r' || *p == '\0') break;
      type = (int) *p++;
      while (isblank(*p)) ++p;
      i = 0;
      while (!isspace(*p)) symbol[i++] = *p++;
      symbol[i] = '\0';
      if (type == 'D') ++nsym;
      /* printf("  %c %s\n", type, symbol); */
      /* symbol already in table? */
      for (k = 0; k < total; ++k) {
        if (strcmp(symtab[k].sym, symbol) == 0) break;
      }
      if (k == total) {
        /* new symbol */
        strcpy(symtab[k].sym, symbol);
        symtab[k].type = type;
        symtab[k].ref = type;
        if (type == 'U') {
          strcpy(symtab[k].refmod, module);
        } else {
          symtab[k].refmod[0] = '\0';
        }
        ++total;
      } else {
        /* existing, update type */
        if (type == 'D') {
          symtab[k].type = type;
          symtab[k].refmod[0] = '\0';
        } else if (type == 'U') {
          strcpy(symtab[k].refmod, module);
        }
        symtab[k].ref = type;
      }
    }
  }

  nundef = 0;
  for (k = 0; k < total; ++k) {
    if (symtab[k].type == 'U') {
      printf("Symbol %10s undefined, last reference from %10s\n",
             symtab[k].sym, symtab[k].refmod);
      ++nundef;
    } else if (symtab[k].ref == 'U') {
      printf("Symbol %10s back referenced, last from %10s\n",
              symtab[k].sym, symtab[k].refmod);
    }
  }

  printf("%s has %d modules, defining %d symbols, %d undefined.\n",
         argv[1], nmod, nsym, nundef);

  fclose(f);

  return 0;
}
