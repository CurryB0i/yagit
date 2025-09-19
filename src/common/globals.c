#include <stdarg.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "globals.h"
#include "platform.h"

char YAGIT_SRC_DIR[PATH_MAX];
char CURRENT_DIR[PATH_MAX];
char YAGIT_DIR[7] = ".yagit";

const char* folders[] = {
  TOILET,
  SNITCHES,
  LANDMINES,
  USELESS_TRIVIA,
  DIRT
};

const char* files[] = {
  LIMBO,
  YESTERDAY,
  SETTINGS_YOULL_BREAK,
  BRAG_SHEET,
  STUFFED_SNITCHES
};

char (*staged)[PATH_MAX];
size_t staged_count = 0;
char (*untracked)[PATH_MAX];
size_t untracked_count = 0;

void build_path(char* buffer, int n, ...) {
  va_list args;
  va_start(args, n);

  strncpy(buffer, va_arg(args, const char* ), PATH_MAX);

  for(int i=1; i<n; i++) {
    int index = strlen(buffer);
    if(buffer[index-1] != PATH_SEP)
      buffer[index] = PATH_SEP;
    buffer[++index] = '\0';
    const char *piece = va_arg(args, const char *);
    strncat(buffer, piece, PATH_MAX - strlen(buffer) - 1);
  }
  va_end(args);
}

