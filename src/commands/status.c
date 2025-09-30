#include "status.h"
#include "globals.h"
#include "limbo.h"
#include "platform.h"
#include "utils.h"
#include <dirent.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

size_t staged_count = 0;
size_t unstaged_count = 0;
size_t untracked_count = 0;
size_t visited_count = 0;
size_t untracked_cap = 8;
char (*staged)[PATH_MAX];
char (*unstaged)[PATH_MAX];
char (*untracked)[PATH_MAX];

bool modified(LimboEntry *entry) {
  struct stat st;
  char full_path[PATH_MAX];
  build_path(full_path, 2, YAGIT_SRC_DIR, entry->path);
  if(stat(full_path, &st) == -1) {
    printf("poof magic");
    return true;
  }
  if(st.st_size != entry->fileSize) return true;
  uint64_t calc_mtime = ((uint64_t) ST_MTIM_SEC(st) << 32) | ST_MTIM_NSEC(st);
  uint64_t act_mtime = ((uint64_t) entry->mtime_sec << 32 ) | entry->mtime_nsec;
  if(calc_mtime != act_mtime) return true;
  return false;
}

bool exists(const char (*array)[PATH_MAX], size_t count, const char* file) {
  for(size_t i=0; i<count; i++) {
    if(strcmp(array[i], file) == 0) 
      return true;
  }
  return false;
}

bool visited(const char* dir_path) {
  for(size_t i=0; i<staged_count; i++) {
    if(strncmp(staged[i], dir_path, strlen(dir_path)) == 0) 
      return true;
  }

  for(size_t i=0; i<unstaged_count; i++) {
    if(strncmp(unstaged[i], dir_path, strlen(dir_path)) == 0) 
      return true;
  } 

  return false;
}

bool recursively_empty(const char* dir_path) {
  DIR *dir = opendir(dir_path);
  if(dir == NULL) return true;
  struct dirent* entry;
  struct stat st;

  while((entry = readdir(dir)) != NULL) {
    if((entry->d_name[0] == '.' && entry->d_name[1] == '\0') ||
        entry->d_name[0] == '.' && entry->d_name[1] == '.' && entry->d_name[2] == '\0') {
      continue;
    }

    char entry_path[PATH_MAX];
    build_path(entry_path, 2, dir_path, entry->d_name);
    if(stat(entry_path, &st) == -1) return true;

    if(S_ISDIR(st.st_mode)) {
      return recursively_empty(entry_path);
    }

    if(S_ISREG(st.st_mode)) {
      return false;
    }
  }

  return true;
}

bool tracked(const char* entry) {
  for(size_t i=0; i<staged_count; i++) {
    if(strcmp(staged[i], entry) == 0)
      return true;
  }

  for(size_t i=0; i<unstaged_count; i++) {
    if(strcmp(unstaged[i], entry) == 0)
      return true;
  }

  return false;
}

void walk_dir(const char* path) {
  DIR *dir;
  struct dirent *entry;
  struct stat st;
  if((dir = opendir(path)) == NULL) return;
  while((entry = readdir(dir)) != NULL) {
    if((entry->d_name[0] == '.' && entry->d_name[1] == '\0') ||
        (entry->d_name[0] == '.' && entry->d_name[1] == '.' && entry->d_name[2] == '\0') ||
        strcmp(entry->d_name, YAGIT_DIR) == 0) {
      continue;
    }

    char entry_path[PATH_MAX];
    build_path(entry_path, 2, path, entry->d_name);
    if(stat(entry_path, &st) == -1) continue;
    if(S_ISDIR(st.st_mode)) {
      if(recursively_empty(entry_path))
        continue;

      if(visited(entry_path + strlen(YAGIT_SRC_DIR) + 1)) {
        walk_dir(entry_path);
      } else {
        if(untracked_cap == untracked_count) {
          untracked_cap *= 2;
          untracked = realloc(untracked, untracked_cap * PATH_MAX);
        }
        snprintf(untracked[untracked_count++], PATH_MAX, "%s%c", entry_path + strlen(YAGIT_SRC_DIR) + 1, PATH_SEP);
      }
    }

    if(S_ISREG(st.st_mode)) {
      if(tracked(entry_path + strlen(YAGIT_SRC_DIR) + 1))
        continue;

      if(untracked_cap == untracked_count) {
        untracked_cap *= 2;
        untracked = realloc(untracked, untracked_cap * PATH_MAX);
      }
      strncpy(untracked[untracked_count++], entry_path + strlen(YAGIT_SRC_DIR) + 1, PATH_MAX);
    }
  }
}

int status_command() {
  look_at_commit();
  staged = malloc(limbo.header.entry_count * sizeof(*staged));
  unstaged = malloc(limbo.header.entry_count * sizeof(*unstaged));
  untracked = malloc(untracked_cap * sizeof(*untracked));

  for(size_t i=0; i<limbo.header.entry_count; i++) {
    if(modified(&limbo.entries[i])) {
      strncpy(unstaged[unstaged_count++], limbo.entries[i].path, PATH_MAX);
    } else {
      strncpy(staged[staged_count++], limbo.entries[i].path, PATH_MAX);
    }
  }
 
  walk_dir(YAGIT_SRC_DIR);

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

  free(staged);
  free(unstaged);
  free(untracked);
  return 0;
}
