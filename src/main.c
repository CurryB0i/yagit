#include "commands.h"
#include "commit.h"
#include "utils.h"
#include "globals.h"
#include <stdio.h>
#include <string.h>

int main(int argc,char* argv[]) {
  if(argc < 2) {
    printf("What, u wanted to say hi? bitch!\n");
    return 0;
  } 

  char* command = argv[1];
  if(strcmp(command, "init") == 0){
    return init_command(argc ,argv);
  }

  if(is_yagit_repo() != 1) {
    print_error("If u not invited, dont ask to gooooo - duke dennis.");
    return 1;
  }

  init();
  int status = 1;
  
  if(strcmp(command, "add") == 0) {
    status = add_command(argc, argv);
  } else if (strcmp(command, "status") == 0) {
    status = status_command(argc, argv);
  } else if (strcmp(command, "commit") == 0) {
    status = commit_command(argc, argv);
  } else if (strcmp(command, "log") == 0) {
    status = log_command(argc, argv);
  } else if (strcmp(command, "cat-file") == 0) {
    status = cat_file_command(argc, argv);
  } else if (strcmp(command, "kill") == 0) {
    status = kill_command(argc, argv);
  } else {
    printf(RED "\nI want u to take '%s' and get the fuck out of here now, u nutsack.\n" RESET,command);
  }

  destruct();
  return status;
}
