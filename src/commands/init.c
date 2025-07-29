#include "../include/init.h"
#include "../platform/platform.h"
#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>

struct stat st = {0};

int init_command(int argc, char* argv[]) {

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
