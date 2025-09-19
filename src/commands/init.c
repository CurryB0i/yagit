#include "init.h"
#include "platform.h"
#include "globals.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

int init_command(int argc, char* argv[]) {
  struct stat st = {0};
  if(stat(".yagit",&st) == -1) {
    int status = MKDIR(".yagit",0700);

    if(GETCWD(YAGIT_SRC_DIR,sizeof(YAGIT_SRC_DIR)) == NULL) {
      perror("Can’t fetch cwd, are you lost, u stupid bitch?");
      return 1;
    }

    if(status == 0) {

#ifdef _WIN32 
      if(!SetFileAttributes(".yagit",FILE_ATTRIBUTE_HIDDEN)) {
        DWORD err = GetLastError();
        fprintf(stderr,"Nah fam, u on windows i cant hide my folder, thats yo fault : %lu\n",err);
        return 1;
      }
#endif

      for(int i=0; i<NO_OF_FOLDERS; i++) {
        char folderPath[PATH_MAX];
        snprintf(folderPath, sizeof(folderPath), ".yagit/%s", folders[i]);

        status = MKDIR(folderPath,0700);
        if(status != 0) {
          perror("yagit out, i cant even create a fcking dir in this mf, how do u expect me to version control : ");
          return 1;
        }    
      }

      //exclude limbo from file creations while initing
      for(int i=1; i<NO_OF_FILES; i++) {
        char filePath[PATH_MAX];
        snprintf(filePath, sizeof(filePath), ".yagit/%s", files[i]);

        FILE* fp = fopen(filePath,"w");
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
