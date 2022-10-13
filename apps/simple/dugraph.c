/*  Date: 17 Dec 87 02:33:54 GMT
 *  Organization: Cullinet Software, Westwood, MA
 *  Dale Worley    Cullinet Software      ARPA: culdev1!drw@eddie.mit.edu
 *  UUCP: ...!seismo!harvard!mit-eddie!culdev1!drw
 */

/*
 * Program to make a pretty graph out of a du report
 * Use it like
 *
 *  du . | dugraph
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* number of lines the listing should occupy */
int length = 60;

/* message for suppressed directories */
#define SUPPRESSED  "(etc.)"

/* format of a tree node */
struct node {
    struct node *lson;      /* left son */
    struct node *rbrother;  /* right brother */
    unsigned long size;     /* size of directory in kbytes */
    int loc;                /* location we will print it at */
    int print_col;          /* column to print name in */
    int print_limit;        /* location we can't print on or after */
    int last;               /* are we last son of our father? */
    char name[1];           /* name */
};

/* root of the tree */
struct node *root = NULL;

/* total size of things listed */
unsigned long total_size;

/* current line number we are on (0-origin) */
int current_line = 0;

/* list of where to put bars */
int bar_list[50];

/* number of bars in the list */
int bar_count = 0;

/* declare functions */
void read_input();
struct node *insert_in_tree();
void dfs();
void dfs1();
void missing_sizes();
void sort();
void calc_loc();
void blank();
void mark_last();
void calc_pc();
void output();
void position();

int main() {
  struct node *t;  /* scratch */

  /* read the input and form a tree */
  read_input();
  root->size = 0;

  /* put sizes on entries that have none */
  dfs(NULL, missing_sizes);

  /* sort each directory */
  dfs(sort, NULL);

  /* calculate the total size */
  total_size = 0;

  for (t = root->lson; t != NULL; t = t->rbrother)
    total_size += t->size;

  /* calculate the location of each directory */
  /* blank out subdirectories that get scrunched together at the bottom */
  root->print_limit = length;
  dfs(calc_loc, blank);

  /* print out the tree */
  for (t = root->lson; t != NULL; t = t->rbrother) {
    /* mark the last son of each directory */
    /* figure out the print columns */
    t->print_col = 0;
    dfs1(calc_pc, mark_last, t);
    dfs1(output, NULL, t);
  }

  /* put blank space at end */
  position(length);
}

/* read input and form a tree */
void read_input() {
  unsigned long  size;    /* size read from input */
  char     name[100];     /* directory name read from input */

  /* make the dummy node at the top of the tree */
  root = (struct node *) calloc(1, sizeof (struct node));
  if (!root) {
    fprintf(stderr, "out of memory\n");
    exit(1);
  }
  root->name[0] = '\0';
  root->lson = NULL;

  /* read the next line of input */
  while (fscanf(stdin, "%ld %s\n", &size, name) == 2 /*!= EOF*/) {
    /* insert (or find) the directory in the tree and save its size */
    insert_in_tree(name)->size = size;
  }
}

/* insert (or find) a directory in the tree */
struct node *insert_in_tree(name)
  char *name;       /* name of the directory */
{
  struct node *t;   /* pointer for searching down through tree */
  char   *np;       /* points to next part of directory name to be examined */
  struct node *t1;  /* scratch pointer */
  char   *np1;      /* scratch pointer */

  /* read through the name, one directory-part at a time, and hunt
   * down the tree, constructing nodes as needed */
  for (t = root, np = name; np != NULL; np = np1) {
    /* extract the next directory-part */
    if ((np1 = strchr(np, '/')) != NULL) {
      /* we found a slash, replace it with a null, and position
       * np1 to point to the remainder of the name */
      *np1++ = '\0';
    }
    /* else */
      /* we found no slash, so we are at the end of the name
       * np1 has been set to NULL for us by strchr */

    /* search the sons of this node for a node with the proper name */
    for (t1 = t->lson; t1 != NULL && strcmp(t1->name, np) != 0;
        t1 = t1->rbrother)
      ;

    /* did we find one? */
    if (t1 != NULL)
      /* yes, go to it */
      t = t1;
    else {
      /* no, make one */
      t1 = (struct node *) calloc(1, sizeof(struct node) + strlen(np));
      if (!t1) {
        fprintf(stderr, "out of memory\n");
        exit(1);
      }
      strcpy(t1->name, np);
      t1->lson = NULL;
      t1->rbrother = NULL;
      t1->size = 0;
      /* insert it in tree */
      t1->rbrother = t->lson;
      t->lson = t1;
      t = t1;
    }
  }

  return t;
}

/* depth-first-search routine */
void dfs(pre_routine, post_routine)
  void (*pre_routine)();  /* routine to execute before scanning descendants */
  void (*post_routine)(); /* routine to execute after scanning descendants */
{
  dfs1(pre_routine, post_routine, root);
}

/* depth-first-search service routine */
void dfs1(pre_routine, post_routine, t)
  void (*pre_routine)();  /* routine to execute before scanning descendants */
  void (*post_routine)(); /* routine to execute after scanning descendants */
  struct node *t;    /* node to operate on */
{
  struct node *t1;   /* scratch pointer */

  /* if it exists, execute the pre-routine */
  if (pre_routine != NULL)
    pre_routine(t);

  /* call self on sons of this node */
  for (t1 = t->lson; t1 != NULL; t1 = t1->rbrother)
    dfs1(pre_routine, post_routine, t1);

  /* if it exists, execute the post-routine */
  if (post_routine != NULL)
    post_routine(t);
}

/* add missing sizes */
void missing_sizes(t)
  struct node *t;
{
  struct node *t1;    /* scratch pointer */
  unsigned long s;    /* scratch */

  if (t->size == 0) {
    /* size is missing, we have to calcuate it */
    s = 0;
    for (t1 = t->lson; t1 != NULL; t1 = t1->rbrother)
      s += t1->size;
    t->size = s;
  }
}

/* sort the directories under a directory */
void sort(t)
  struct node  *t;
{
  struct node  *p1, *p2, *p3, *pp;    /* scratch pointers */
  int    nodes, n;                    /* scratch */

  /* count the number of nodes */
  nodes = 0;
  for (p1 = t->lson; p1 != NULL; p1 = p1->rbrother) nodes++;

  /* just a simple and inefficient bubble sort */
  for (n = 1; n < nodes; n++)
    for (p1 = NULL, p2 = t->lson, p3 = p2->rbrother; p3 != NULL;
         p1 = p2, p2 = p3, p3 = p3->rbrother) {
      if (p2->size < p3->size) {
        /* exchange the nodes p2 and p3 */
        pp = p3->rbrother;
        p3->rbrother = p2;
        p2->rbrother = pp;
        if (p1 != NULL)
          p1->rbrother = p3;
        else
          t->lson = p3;
        /* exchange the values of p2 and p3 */
        pp = p2;
        p2 = p3;
        p3 = pp;
      }
    }
}

/* calculate the print location */
void calc_loc(t)
  struct node  *t;
{
  unsigned long  cs;      /* scratch */
  struct node  *t1, *t2;  /* scratch pointers */
  int    print_limit;     /* location next directory after t will be printed */

  if (t == root)
    cs = 0;
  else {
    /* figure out how much is in the directory itself */
    for (t1 = t->lson, cs = 0; t1 != NULL; t1 = t1->rbrother) {
      cs += t1->size;
    }
    /* cs is the size accounted for by subdirectories */
    cs = t->size - cs;
  }

  /* cs is the size of the files in the directory itself */
  /* convert cs to lines */
  cs = cs * length / total_size + t->loc;

  /* calculate where next directory after t will be */
  print_limit = t->print_limit;

  /* assign locations */
  for (t1 = t->lson, t2 = NULL; t1 != NULL; t2 = t1, t1 = t1->rbrother) {
    /* make sure we don't run into next directory */
    if (cs >= print_limit) {
      cs = print_limit-1;
    }
    t1->loc = cs;
    if (t2 != NULL)
      t2->print_limit = cs;
    cs += t1->size * length / total_size;
  }

  if (t2 != NULL)
    t2->print_limit = print_limit;
}

/* figure out which directories to blank out */
void blank(t)
  struct node  *t;
{
  struct node  *t1, *t2, *t3;    /* loop pointers */

  /* return if there aren't at least two sons */
  if (t->lson == NULL || t->lson->rbrother == NULL)
    return;

  for (t1 = NULL, t2 = t->lson, t3 = t2->rbrother; t3 != NULL;
       t1 = t2, t2 = t3, t3 = t3->rbrother)
    if (t2->loc == t3->loc) {
      /* replace t1 and succeeding nodes with "(etc.)" */
      t3 = (struct node *) calloc(1, sizeof (struct node) +
                                     sizeof (SUPPRESSED) - 1);
      if (!t3) {
        fprintf(stderr, "out of memory\n");
        exit(1);
      }
      strcpy(t3->name, SUPPRESSED);
      t3->lson = t3->rbrother = NULL;
      t3->loc = t2->loc;
      if (t1 == NULL)
        t->lson = t3;
      else
        t1->rbrother = t3;
    }
}

/* mark the last son of each directory */
void mark_last(t)
  struct node  *t;
{
  struct node  *t1, *t2;  /* scratch pointers */

  t->last = 0;
  for (t1 = t->lson, t2 = NULL; t1 != NULL; t2 = t1, t1 = t1->rbrother)
    ;

  if (t2 != NULL) t2->last = 1;
}

/* calculate the print columns */
void calc_pc(t)
  struct node  *t;
{
  struct node  *t1;    /* scratch pointer */
  int    c;            /* column suns will be printed in */

  c = t->print_col + strlen(t->name) + 5;
  for (t1 = t->lson; t1 != NULL; t1 = t1->rbrother) t1->print_col = c;
}

/* write the output */
void output(t)
  struct node  *t;
{
  position(t->loc);
  printf("--%s%s", t->name, (t->lson != NULL ? "--+" : ""));

  /* remove the bar for our father if we are the last son */
  if (t->last) bar_count--;

  /* add the location of the bar to the bar list if we have a son */
  if (t->lson != NULL) {
    bar_list[bar_count] = t->print_col + strlen(t->name) + 5 - 1;
    bar_count++;
  }
}

/* position to a specific line */
void position(line)
  int  line;    /* line number */
{
  int  i;    /* counts through the bar list */
  int  j;    /* current column number */

  /* for every line we need to go down */
  for (; current_line < line; current_line++) {
    putchar('\n');
    /* print the bars for this line */
    j = 0;
    for (i = 0; i < bar_count; i++) {
      for (; j < bar_list[i]; j++) putchar(' ');
      if (current_line == line-1 && i == bar_count-1)
        putchar('+');
      else
        putchar('|');
      j++;
    }
  }
}
