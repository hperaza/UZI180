/*
 * tree - a utility to show directory tree structure
 *
 * Copyright (C) 1993 Jeff Tranter (tranter@mitel.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

#define lstat stat

/* maximum number of directories that can be specified with -x options */
#define MAX_DIRS 100

/*
 * Global command option flags. Set by command line options.
 * Initialized here to defaults.
 */
static char *progName = "tree";               /* name of program (used in error messages etc...) */
static char *indentString = "  ";             /* string to indent with */
static int maxIndent = 100;                   /* maximum dir levels to descend */
static int verboseFlag = 0;                   /* verbose output */
static int allFlag = 0;                       /* display all files */
static int fullFlag = 0;                      /* display full pathnames */
static int linkFlag = 0;                      /* follow symbolic links */
static char *ignoreDirs[MAX_DIRS] = { ".", ".." }; /* directories to ignore */
static int ignoreIndex = 2;                   /* next available index into above array */

/*
 * Utility function to print indent string for each indent level.
 */

static void indent(int level) {
  while (level > 0) {
    printf("%s", indentString);
    level--;
  }
}

/*
 * Utility function to return basename of a pathname.
 */
static char *basename(char *name) {
  char *pos = strrchr(name, '/');
  if (pos == NULL)
    return name;
  else
    return pos+1;
}

/*
 * Display command usage on standard error and exit.
 */
static void usage(void) {
  fprintf(stderr, 
	  "usage: %s [-i<string>] [-m<number>] [-x<dir>] [-vfpl] [pathname]\n\n"
	  "options:\n"
	  " -i <string>   specify indent string\n"
	  " -m <number>   specify maximum indent level\n"
          " -x <dir>      exclude specific directory (can be repeated)\n"
          " -v            verbose; display files that cannot be opened etc..\n"
	  " -f            display all files (not just dirs)\n"
	  " -p            show full pathnames\n"
	  " -l            follow symbolic links\n\n"
	  "The default is to indent with two spaces, descend a maximum of 100\n"
	  "levels deep, suppress errors for unreadable files, only display\n"
	  "basenames of directories, and exclude directories `.' and `..'.\n"
	  "The default pathname is the current working directory.\n\n"
	  "The -x option can use absolute pathnames (e.g. -x /proc) or relative\n"
	  "names (e.g. -x RCS).\n",
	  progName);
  exit(1);
}

/*
 * Display directory tree starting at "dir" and indent level.
 * Recursively calls self to display sub-directories.
 */

static void display(char *dirName, int indentLevel) {
  int status, i, ignore;
  DIR *dirPtr;
  struct direct *direntPtr;
  struct stat statbuf;
  char buf[1024];
  char fullName[1024];

  /* exit if maximum indent level reached */
  if (indentLevel >= maxIndent)
    return;

  /* open directory */
  dirPtr = opendir(dirName);

  /* error if could not open directory */
  if (dirPtr == 0) {
    /* display error if verbose flag set or if this is the first directory */  
    if (verboseFlag || (indentLevel == 0)) {
      sprintf(buf, "%s: %s", progName, dirName);
      perror(buf);
    }
    return;
  }

  /* display directory name */
  indent(indentLevel);
  /* display full pathname is fullFlag set or if this is the first directory */
  if (indentLevel == 0 || fullFlag)
    printf("%s\n", dirName);
  else 
    printf("%s/\n", basename(dirName));

  /* read each directory entry */
  while ((direntPtr = readdir(dirPtr))) {

    /* calculate full pathname: dirName + "/" + directory entry */
    strcpy(fullName, dirName);
    /* only add trailing slash if there isn't one already */
    if (dirName[strlen(dirName)-1] != '/')
      strcat(fullName, "/");
    strcat(fullName, direntPtr->d_name);

    /* See if this is a directory that should be ignored. If ignore
     * pattern starts with '/', compare to full pathname, otherwise
     * compare to directory entry.
     */
    ignore = 0;
    for (i = 0 ; (i < ignoreIndex) && !ignore ; i++)
      if (ignoreDirs[i][0] == '/') {
	/* absolute path */
	if (strcmp(fullName, ignoreDirs[i]) == 0)
	  ignore = 1;
      }	else {
	/* relative path */
	if (strcmp(direntPtr->d_name, ignoreDirs[i]) == 0) {
	  ignore = 1;
	}
      }
    
    if (!ignore) {

      /* find out if this is a sub-directory */
      if (linkFlag)
	status = stat(fullName, &statbuf);
      else
	status = lstat(fullName, &statbuf);
      if (status == -1) {
	if (verboseFlag) {
	  sprintf(buf, "%s: unable to stat %s", progName, fullName);
	  perror(buf);
	}
      }

      /* if file is directory, call display recursively */
      if (S_ISDIR(statbuf.st_mode)) {
	display(fullName, indentLevel+1);
      } else {
	/* if not directory, and -f flag set, display file */
	if (allFlag) {
	  indent(indentLevel+1);
	  if (fullFlag) 
	    printf("%s\n", fullName);
	  else
	    printf("%s\n", direntPtr->d_name);
	}
      }
    }
  }
  closedir(dirPtr);
}

/*
 * Main program. Parse arguments and call display().
 */

int main(int argc, char **argv) {
  int i, c;
  char *getcwd();

  progName = argv[0];

  /* get options, if any, and set flags */
  for (i = 1; i < argc; ++i) {
    if (argv[i][0] == '-')
      c = argv[i][1];
    else
      break;

    switch (c) {
    case 'i':
      indentString = argv[++i];
      break;

    case 'm':
      maxIndent = atoi(argv[++i]);
      break;

    case 'v':
      verboseFlag = 1;
      break;

    case 'f':
      allFlag = 1;
      break;

    case 'x':
      ignoreDirs[ignoreIndex] = argv[++i];
      ignoreIndex++;
      /* display error message and exit if maximum number reached */
      if (ignoreIndex >= MAX_DIRS) {
	fprintf(stderr, "%s: too many -x options, use fewer options or increase MAX_DIRS and recompile\n", progName);
	exit(1);
      }
      break;

    case 'p':
      fullFlag = 1;
      break;

    case 'l':
      linkFlag = 1;
      break;

    default:
    case '?':
      usage();
      break;

    }
  }

  /* get arguments after options */
  switch (argc - i) {
  case 0: /* no path on command line, use current dir */
    display(getcwd(NULL, 1024), 0);
    break;

  case 1: /* path given on command line */
    display(argv[i], 0);
    break;

  default:
    fprintf(stderr, "%s: too many pathname arguments\n", progName);
    usage();
    break;
  }

  return (0);
}
