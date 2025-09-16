#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <sys/stat.h>
#include "globals.h"
#include "platform.h"
#include "check.h"

int is_yagit_repo() {
  DIR *dir;
  struct dirent *entry;
  char path[PATH_MAX];
  int found = 0;
  struct stat st;

  if(GETCWD(path, sizeof(path)) == NULL) {
    perror("Cant fetch cwd: ");
    return -1;
  }

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


      if(S_ISDIR(st.st_mode) && strcmp(entry->d_name,".yagit")==0) {
        snprintf(YAGIT_SRC_DIR,sizeof(YAGIT_SRC_DIR),"%s",path);
        found = 1;
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

  for(int i=0; i<NO_OF_FILES; i++) {
    char test[PATH_MAX];
    snprintf(test, sizeof(test), "%s%c.yagit%c%s", path, PATH_SEP, PATH_SEP, files[i]);
    if(access(test, F_OK) == -1) {
      printf("Where my '%s' at, nah u fucked up, just give up now.\n",files[i]);
    }
  }

  return 1;
}
