#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include "globals.h"
#include "platform.h"
#include "initialize.h"

void init() {
  size_t cap = 8;
  untracked = malloc(cap * sizeof(*untracked));
  staged = malloc(cap * sizeof(*staged));
}

void walkdir(const char* folder_path, size_t *cap) {
  DIR *dir = opendir(folder_path);
  struct dirent *entry;
  struct stat st;

  while((entry = readdir(dir)) != NULL) {
    if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, YAGIT_DIR) == 0) 
      continue;

    char full_path[PATH_MAX];
    build_path(full_path, 2, folder_path, entry->d_name);

    if(stat(full_path, &st) == -1)
      continue;

    if(S_ISDIR(st.st_mode)) {
      walkdir(full_path, cap);
    }

    if(S_ISREG(st.st_mode)) {
      if(untracked_count == *cap) {
        *cap *= 2;
        untracked = realloc(untracked, *cap * sizeof(*untracked));
      }

      bool is_staged = false;
      char relative_path[PATH_MAX];
      strncpy(relative_path, full_path + strlen(YAGIT_SRC_DIR) + 1, sizeof(relative_path));

      for(size_t i=0; i<staged_count; i++) {
        if(strcmp(staged[i], relative_path) == 0) {
          is_staged = true;
          break;
        }
      }

      if(is_staged) continue;
      strncpy(untracked[untracked_count++], relative_path, PATH_MAX);
    }
  }
  closedir(dir);
}

void populate_limbo() {
  size_t cap = 8;
  walkdir(YAGIT_SRC_DIR, &cap); 
}

void read_limbo() {
  char limbo_path[PATH_MAX];
  build_path(limbo_path, 3, YAGIT_SRC_DIR, YAGIT_DIR, LIMBO);
  struct stat st;

  if(stat(limbo_path, &st) == -1) {
    populate_limbo();
    return;
  }

  FILE* limbo_file = fopen(limbo_path, "r");

  char line[256];
  int cap = 8, n = 0;
  untracked = malloc(cap * sizeof(*untracked));
  staged = malloc(cap * sizeof(*staged));

  size_t untracked_count = 0;
  size_t staged_count = 0;
  while(fgets(line, sizeof(line), limbo_file)) {
    if(line[0] == '?' && line[1] == ' ') {
      if(untracked_count == cap) {
        cap *= 2;
        untracked = realloc(untracked, cap * sizeof(*untracked));
      }
      strncpy(untracked[untracked_count++], line, PATH_MAX);
    } else if (line[0] == 'A' && line[1] == ' ') {
      if(staged_count == cap) {
        cap *= 2;
        staged = realloc(staged, cap * sizeof(*staged));
      }
      strncpy(staged[staged_count++], line, PATH_MAX);
    }
  }

  fclose(limbo_file);
}

int is_yagit_repo() {
  DIR *dir;
  struct dirent *entry;
  char path[PATH_MAX];
  bool found = false;
  struct stat st;

  if(GETCWD(path, sizeof(path)) == NULL) {
    perror("Cant fetch cwd: ");
    return -1;
  }
  strncpy(CURRENT_DIR, path, sizeof(CURRENT_DIR));

  while(1) {
    char temp[PATH_MAX];
    snprintf(temp, sizeof(temp), "%s%c", path, PATH_SEP);
    if((dir = opendir(temp)) == NULL) {
      perror("Error opening directory");
      return -1;
    }

    while((entry = readdir(dir)) != NULL) {
      char full_path[PATH_MAX];
      snprintf(full_path, sizeof(full_path), "%s%s", temp, entry->d_name);
      if(stat(full_path, &st) == -1) {
        continue;
      }

      if(S_ISDIR(st.st_mode) && strcmp(entry->d_name, YAGIT_DIR)==0) {
        strncpy(YAGIT_SRC_DIR, path, sizeof(YAGIT_SRC_DIR));
        found = true;
        break;
      }
    }

    if(found) break;

    if(strlen(path) == 2 || strlen(path) == 0) {
      return 0;
    }

    char* slash = strrchr(path,PATH_SEP);
    if(slash) {
      *slash = '\0';
    }
    closedir(dir);
  }

  for(int i=0; i<NO_OF_FOLDERS; i++) {
    char test[PATH_MAX];
    snprintf(test, sizeof(test), "%s%c.yagit%c%s", path, PATH_SEP, PATH_SEP, folders[i]);
    if(access(test, F_OK) == -1) {
      printf("Where my '%s' at, nah u fucked up, just give up now.\n",folders[i]);
    }
  }

  for(int i=1; i<NO_OF_FILES; i++) {
    char test[PATH_MAX];
    snprintf(test, sizeof(test), "%s%c.yagit%c%s", path, PATH_SEP, PATH_SEP, files[i]);
    if(access(test, F_OK) == -1) {
      printf("Where my '%s' at, nah u fucked up, just give up now.\n",files[i]);
    }
  }

  return 1;
}
