#include <string.h>
#include <stdlib.h>
#include "log.h"
#include "utils.h"
#include "object.h"

int print_commit(Commit* c) {
  int idx = c->parent_count - 1;
  for(int idx=c->parent_count-1; idx>=0; idx--) {
    Commit *parent_commit = malloc(sizeof(Commit));
    strcpy(parent_commit->author.type, "author");
    strcpy(parent_commit->committer.type, "committer");
    build_commit(parent_commit, c->parents[idx]);
    print_commit(parent_commit);
    printf("\n%*s\n", (SHA256_DIGEST_SIZE*2+11)/2, "|");
    printf("%*s\n", (SHA256_DIGEST_SIZE*2+11)/2, "v");
  }

  printf("\n");
  for(int i=0; i<SHA256_DIGEST_SIZE*2+11; i++) {
    printf("=");
  }
  int padding = SHA256_DIGEST_SIZE*2 + 9;
  char formatted[1024];

  printf(YELLOW "\n|Commit ");
  print_hash(c->hash);
  printf("|");

  snprintf(formatted, sizeof(formatted), "Author: %s %s", c->author.user.name, c->author.user.email);
  printf(RESET "\n|%-*s|\n",padding, formatted);

  printf("|Date: ");
  print_localtime(c->author.when);
  printf(" ");
  print_tz_offset(c->author.tz_offset_minutes);
  printf("%-*s|", padding-38, "");

  snprintf(formatted, sizeof(formatted), "Committer: %s %s", c->committer.user.name, c->committer.user.email);
  printf(RESET "\n|%-*s|\n",padding, formatted);

  printf("|Date: ");
  print_localtime(c->committer.when);
  printf(" ");
  print_tz_offset(c->committer.tz_offset_minutes);
  printf("%-*s|", padding-38, "");

  printf("\n|%*s|\n", padding, "");
  int lpad, rpad;
  lpad = (padding + strlen(c->message))/2;
  if(strlen(c->message) % 2 == 0) {
    rpad = (padding - strlen(c->message))/2;
  } else {
    rpad = (padding - strlen(c->message))/2 - 1;
  }
  printf(CYAN "|%*s %*s|\n" RESET, lpad, c->message, rpad, "");

  for(int i=0; i<SHA256_DIGEST_SIZE*2+11; i++) {
    printf("=");
  }
  printf("\n");

  return 0;
}

int log_command(int argc, char* argv[]) {
  if(argc != 2) {
    print_error("U brough uninvited visitors. Get out!");
    destruct();
    exit(1);
  }

  if(commit.is_first) {
    printf(CYAN "\nNo Commits yet\n" RESET);
    return 0;
  }

  return print_commit(&commit);
}