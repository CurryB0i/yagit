#include "include/commands.h"
#include <stdio.h>
#include <string.h>

int main(int argc,char* argv[]) {
    if(argc < 2) {
        printf("What, u wanted to say hi? bitch!\n");
        return 0;
    } 

    char* command = argv[1];
    if(strcmp(command,"init")==0){
        return init_command(argc -1 ,argv);
    }

    return 0;
}
