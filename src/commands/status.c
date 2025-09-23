#include "status.h"
#include "globals.h"
#include "limbo.h"
#include "platform.h"
#include "utils.h"
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

bool modified(LimboEntry *entry) {
  struct stat st;
  char full_path[PATH_MAX];
  build_path(full_path, 2, YAGIT_SRC_DIR, entry->path);
  if(stat(full_path, &st) == -1) {
    printf("poof magic");
    return true;
  }
  if(st.st_size != entry->fileSize) return true;
  uint64_t calc_mtime = ((uint64_t) ST_MTIM_SEC(st)) | ST_MTIM_NSEC(st);
  uint64_t act_mtime = ((uint64_t) entry->mtime_sec) | entry->mtime_nsec;
  if(calc_mtime != act_mtime) return true;
  return false;
}

int status_command() {
  char (*staged)[PATH_MAX] = malloc(limbo.header.entry_count * PATH_MAX);
  char (*unstaged)[PATH_MAX] = malloc(limbo.header.entry_count * PATH_MAX);
  size_t staged_count = 0;
  size_t unstaged_count = 0;

  for(size_t i=0; i<limbo.header.entry_count; i++) {
    if(modified(&limbo.entries[i])) {
      strncpy(unstaged[unstaged_count++], limbo.entries[i].path, PATH_MAX);
    } else {
      strncpy(staged[staged_count++], limbo.entries[i].path, PATH_MAX);
    }
  }

  printf("\nUnstaged Files:\n");
  if(unstaged_count == 0) {
    printf(MAGENTA "No Unstaged Files - Yippy :) (Rare wholesome moment cherish it).\n" RESET);
  }
  for(size_t i=0; i<unstaged_count; i++) {
    printf(RED "    %s\n" RESET, unstaged[i]);
  }

  printf("\nStaged Files:\n");
  if(staged_count == 0) {
    printf(MAGENTA "Nobodys on the stage, this is a fcking shit show man, pull ur shit together, bitch ass.\n" RESET);
  }
  for(size_t i=0; i<staged_count; i++) {
    printf(GREEN "    %s\n" RESET, staged[i]);
  }
  return 0;
}
