#pragma once

#include "sha256.h"
#include <stdint.h>
typedef struct {
  char name[256];
  char email[256];
  char date[30];
} Author;

typedef struct {
  bool first;
  uint8_t tree_hash[SHA256_DIGEST_SIZE];
  uint8_t parent_hash[SHA256_DIGEST_SIZE];
  Author author;
  char message[512];
} Commit;

extern Commit commit;

int commit_command(int, char**);
void commit_init();
