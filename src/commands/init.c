#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "init.h"
#include "platform.h"
#include "globals.h"

int init_command(int argc, char* argv[]) {
  struct stat st = {0};
  if(stat(".yagit",&st) == -1) {

    if(GETCWD(YAGIT_SRC_DIR,sizeof(YAGIT_SRC_DIR)) == NULL) {
      perror("Can’t fetch cwd, are you lost, u stupid bitch?");
      return 1;
    }
    char yagit_path[PATH_MAX];
    build_path(yagit_path, 2, YAGIT_SRC_DIR, YAGIT_DIR);
    int status = MKDIR(yagit_path ,0700);

    if(status == 0) {

#ifdef _WIN32 
      if(!SetFileAttributes(".yagit",FILE_ATTRIBUTE_HIDDEN)) {
        DWORD err = GetLastError();
        fprintf(stderr,"Nah fam, u on windows i cant hide my folder, thats yo fault : %lu\n",err);
        return 1;
      }
#endif

      for(int i=0; i<NO_OF_FOLDERS; i++) {
        for(size_t j=0; folders[i][j] != NULL; j++) {
          char folder_path[PATH_MAX];
          if(j == 0) {
            build_path(folder_path, 3, YAGIT_SRC_DIR, YAGIT_DIR, folders[i][0]);
          } else {
            build_path(folder_path, 4, YAGIT_SRC_DIR, YAGIT_DIR, folders[i][0], folders[i][j]);
          }
          status = MKDIR(folder_path, 0700);
          if(status != 0) {
            perror("yagit out, i cant even create a fcking dir in this mf, how do u expect me to version control : ");
            return 1;
          }    
        }
      }

      //exclude limbo from file creations while initing
      for(int i=1; i<NO_OF_FILES; i++) {
        char file_path[PATH_MAX];
        build_path(file_path, 3, YAGIT_SRC_DIR, YAGIT_DIR, files[i]);

        FILE* fp = fopen(file_path,"w");
        if(fp == NULL) {
          perror("yagit out, i cant even create a fcking file in this mf, how do u expect me to version control : ");
          return 1;
        }
        fclose(fp);
      }

      printf("Initialized empty yagit repository in %s\n",YAGIT_SRC_DIR);
      return 0;
    } else {
      perror("yagit out, i cant even create a fcking dir in this mf, how do u expect me to version control : ");
      return 1;
    }
  } else {
    printf("Wtf is the matter with you, I am already doing it!\n");
    return 1;
  }
}
