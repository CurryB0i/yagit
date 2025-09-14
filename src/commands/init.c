#include "init.h"
#include "platform.h"
#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#define NO_OF_FOLDERS 5
#define NO_OF_FILES 5

char* folders[5] = {
  "toilet",
  "snitches",
  "landmines",
  "useless_trivia",
  "dirt",
};

char* files[5] = {
  "yesterday",
  "settings_youll_break",
  "brag_sheet",
  "limbo",
  "stuffed_snitches"
};

int init_command(int argc, char* argv[]) {
  struct stat st = {0};
  if(stat(".yagit",&st) == -1) {
    int status = MKDIR(".yagit",0700);
    char cwd[PATH_MAX];

    if(GETCWD(cwd,sizeof(cwd)) == NULL) {
      perror("Can’t fetch cwd, are you lost?");
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
          fprintf(stderr,"%s : yagit out, i cant even create a fcking dir in this mf, how do u expect me to version control\n"
              ,strerror(errno));
          return 1;
        }    
      }

      for(int i=0; i<NO_OF_FILES; i++) {
        char filePath[PATH_MAX];
        snprintf(filePath, sizeof(filePath), ".yagit/%s", files[i]);

        FILE* fp = fopen(filePath,"w");
        if(fp == NULL) {
          fprintf(stderr,"%s : yagit out, i cant even create a fcking file in this mf, how do u expect me to version control\n"
              ,strerror(errno));
          return 1;
        }
        fclose(fp);
      }

      printf("Initialized empty yagit repository in %s\n",cwd);
      return 0;
    } else {
      fprintf(stderr,"%s : yagit out, i cant even create a fcking dir in this mf, how do u expect me to version control\n",strerror(errno));
      return 1;
    }
  } else {
    printf("Wtf is the matter with you, I am already doing it!\n");
    return 1;
  }
}
