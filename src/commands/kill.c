#include "kill.h"
#include "globals.h"
#include "utils.h"
#include "platform.h"
#include <dirent.h>

void remove_dir(const char* path) {
  DIR* dir = opendir(path);
  struct dirent* entry;
  struct stat st;

  if(!dir) {
    printf("WTF!");
    return;
  }

  while((entry = readdir(dir)) != NULL) {
    if((entry->d_name[0] == '.' && entry->d_name[1] == '\0') ||
      (entry->d_name[0] == '.' && entry->d_name[1] == '.' && entry->d_name[2] == '\0') ) {
      continue;
    }

    char new_path[PATH_MAX];
    build_path(new_path, 2, path, entry->d_name);

    if(STAT(new_path, &st) == -1) {
        printf("delorian");
        continue;
    }

    if(S_ISREG(st.st_mode)) {
      remove(new_path);
    }

    if(S_ISDIR(st.st_mode)) {
      remove_dir(new_path);
    }

  }

  closedir(dir);
  RMDIR(path);
}

int kill_command(int argc, char* argv[]) {
  char path[PATH_MAX];
  build_path(path, 2, YAGIT_SRC_DIR, YAGIT_DIR);
  snprintf(path, sizeof(path), "%s%c%s", YAGIT_SRC_DIR, PATH_SEP, YAGIT_DIR);
  remove_dir(path);
  printf(BLUE "\nGoodBye");
  for(int i=0; i<7; i++) {
    printf(".");
    SLEEP(500);
  }
  printf(RESET "\n");
}