#include "commands.h"
#include "utils.h"
#include "globals.h"
#include <stdio.h>
#include <string.h>

int main(int argc,char* argv[]) {
  init();
  if(argc < 2) {
    printf("What, u wanted to say hi? bitch!\n");
    return 0;
  } 

  char* command = argv[1];
  if(strcmp(command,"init")==0){
    return init_command(argc-1 ,argv);
  }

  if(is_yagit_repo() != 1) {
    printf("if u not invited, dont ask to gooooo - duke dennis.");
    return 1;
  }

  if(strcmp(command,"add")==0){
    return add_command(argc-1, argv);
  }

  printf("i want u to take '%s' and get the fuck out of here now, u nutsack.",command);
  return 1;
}
