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

    char* p = strrchr(path,PATH_SEP);
    if(p) {
      *p = '\0';
    }
    closedir(dir);
  }

  for(int i=0; i<NO_OF_FOLDERS; i++) {
    char test[PATH_MAX];
    build_path(test, 3, path, YAGIT_DIR, folders[i]);
    if(access(test, F_OK) == -1) {
      printf("Where my '%s' at, nah u fucked up, just give up now.\n",folders[i]);
    }
  }

  for(int i=1; i<NO_OF_FILES; i++) {
    char test[PATH_MAX];
    build_path(test, 3, path, YAGIT_DIR, files[i]);
    if(access(test, F_OK) == -1) {
      printf("Where my '%s' at, nah u fucked up, just give up now.\n",files[i]);
    }
  }

  return 1;
}

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
