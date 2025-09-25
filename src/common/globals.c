#include <stdarg.h>
#include <limits.h>
#include <stddef.h>
#include "globals.h"
#include "limbo.h"
#include "object.h"

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

void init() {
  limbo_init();
  object_init();
}
