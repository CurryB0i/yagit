#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include "globals.h"
#include "platform.h"
#include "utils.h"

void init() {
  visited = malloc(visited_cap * sizeof(*visited));
  untracked = malloc(untracked_cap * sizeof(*untracked));
  staged = malloc(staged_cap * sizeof(*staged));
}

void crlf_to_lf(char *buffer, size_t *buffer_len) {
  size_t j = 0;
  for (size_t i = 0; i < *buffer_len; i++) {
    if (buffer[i] == '\r' && i + 1 < *buffer_len && buffer[i + 1] == '\n') {
      continue;
    }
    buffer[j++] = buffer[i];
  }
  *buffer_len = j;
}

bool starts_with(const char *str, const char *prefix) {
  size_t len_prefix = strlen(prefix);
  size_t len_str = strlen(str);
  return len_str < len_prefix ? false : strncmp(str, prefix, len_prefix) == 0;
}

void populate_dir_entries(const char* folder_path) {
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

    char relative_path[PATH_MAX];
    strncpy(relative_path, full_path + strlen(YAGIT_SRC_DIR) + 1, sizeof(relative_path));
    if(S_ISDIR(st.st_mode)) {
      snprintf(untracked[untracked_count++], PATH_MAX, "%s%c" ,relative_path, PATH_SEP);
    }

    if(S_ISREG(st.st_mode)) {
      if(untracked_count == untracked_cap) {
        untracked_cap *= 2;
        untracked = realloc(untracked, untracked_cap * sizeof(*untracked));
      }

      bool is_staged = false;

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
  char limbo_path[PATH_MAX];
  build_path(limbo_path, 3, YAGIT_SRC_DIR, YAGIT_DIR, LIMBO);
  struct stat st;

  if(stat(limbo_path, &st) == -1) {
    populate_dir_entries(YAGIT_SRC_DIR);
    return;
  }

  FILE* limbo_file = fopen(limbo_path, "r");

  char line[256];
  size_t n = 0;

  while(fgets(line, sizeof(line), limbo_file)) {
    //crlf -> lf
    bool flag = false;
    line[strcspn(line, "\r\n")] = '\0';
    if(line[0] == '?' && line[1] == ' ') {
      if(untracked_count == untracked_cap) {
        untracked_cap *= 2;
        untracked = realloc(untracked, untracked_cap * sizeof(*untracked));
      }
      strncpy(untracked[untracked_count++], line+2, PATH_MAX);
    } else if (line[0] == 'A' && line[1] == ' ') {
      if(staged_count == staged_cap) {
        staged_cap *= 2;
        staged = realloc(staged, staged_cap * sizeof(*staged));
      }
      strncpy(staged[staged_count++], line+2, PATH_MAX);
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
