#include "globals.h"
#include <stdio.h>

char YAGIT_SRC_DIR[PATH_MAX];

const char* folders[] = {
  TOILET,
  SNITCHES,
  LANDMINES,
  USELESS_TRIVIA,
  DIRT
};

const char* files[] = {
  YESTERDAY,
  SETTINGS_YOULL_BREAK,
  BRAG_SHEET,
  LIMBO,
  STUFFED_SNITCHES
};

void build_path(char* buffer, size_t size, const char* name) {
  snprintf(buffer, size, "%s\\.yagit\\%s", YAGIT_SRC_DIR, name);
}
