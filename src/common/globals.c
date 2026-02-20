#include <stdarg.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include "globals.h"
#include "limbo.h"
#include "object.h"
#include "commit.h"

char YAGIT_SRC_DIR[PATH_MAX];
char CURRENT_DIR[PATH_MAX];
char YAGIT_DIR[7] = ".yagit";

const char *folders[][PATH_MAX] = {
  { TOILET, NULL },
  { SNITCHES, HEADS, NULL },
  { LANDMINES, NULL },
  { USELESS_TRIVIA, NULL },
  { DIRT, NULL }
};

const char* files[] = {
  LIMBO,
  YESTERDAY,
  SETTINGS_YOULL_BREAK,
  BRAG_SHEET,
  STUFFED_SNITCHES
};

void init() {
  limbo_init();
  tree_init();
  commit_init();
}