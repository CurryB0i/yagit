#include <sys/stat.h>
#include <limits.h>
#include <stdio.h>
#include "status.h"
#include "globals.h"
#include "utils.h"

int status_command() {
  struct stat st;
  char limbo_path[PATH_MAX];
  build_path(limbo_path, 3, YAGIT_SRC_DIR, YAGIT_DIR, LIMBO);
  populate_limbo();

  printf("Changes to be committed:\n");
  for(int i=0; i<staged_count; i++) {
    printf(GREEN "\tnew file:\t%s\n" RESET, staged[i]);
  }
  printf("\nUntracked files:\n");
  for(int i=0; i<untracked_count; i++) {
    printf(RED "\t%s\n" RESET, untracked[i]);
  }

  return 0;
}
