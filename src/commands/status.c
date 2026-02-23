#include "stage.h"
#include "utils.h"
#include <stdlib.h>

int status_command(int argc, char* argv[]) {
  if(argc != 2) {
    print_error("U brough uninvited visitors. Get out!");
    destruct();
    exit(1);
  }

  set_stage();

  if(unstaged_count != 0) {
    printf("\nUnstaged Files:\n");
    for(size_t i=0; i<unstaged_count; i++) {
      printf(RED "    %s\n" RESET, unstaged[i]);
    }
  }

  if(staged_count != 0) {
    printf("\nStaged Files:\n");
    for(size_t i=0; i<staged_count; i++) {
      printf(GREEN "    %s\n" RESET, staged[i]);
    }
  }

  if(untracked_count != 0) {
    printf("\nUntracked Files:\n");
    for(size_t i=0; i<untracked_count; i++) {
      printf(RED "    %s\n" RESET, untracked[i]);
    }
  }

  if(unstaged_count == 0 && staged_count == 0 && untracked_count == 0) {
    printf(CYAN "\nNo Changes\n" RESET);
  }

  destroy_stage();
  return 0;
}
