#pragma once
#include "sha256.h"
#include "globals.h"
#include "config.h"
#include <limits.h>
#include <stdint.h>
#include <sys/types.h>
#include <time.h>

typedef enum {
  OBJ_BLOB = 0,
  OBJ_TREE = 1,
  OBJ_COMMIT = 2
} ObjectType;

typedef struct {
  char type[12];
  User user;
  time_t when;
  int tz_offset_minutes;
} Identity;

typedef struct {
  Mode_t mode;
  char name[PATH_MAX];
  uint8_t hash[SHA256_DIGEST_SIZE];
} Blob;

typedef struct {
  Mode_t mode;
  char name[PATH_MAX];
  uint8_t hash[SHA256_DIGEST_SIZE];
  size_t object_count;
  size_t object_capacity;
  struct Object **objects;
} Tree;

typedef struct {
  bool is_first;
  uint8_t hash[SHA256_DIGEST_SIZE];
  uint8_t tree_hash[SHA256_DIGEST_SIZE];
  size_t parent_count;
  uint8_t (*parents)[SHA256_DIGEST_SIZE];
  Identity author;
  Identity committer;
  char* message;
} Commit;

typedef struct Object {
  ObjectType type;
  union {
    Blob blob;
    Tree tree;
    Commit commit;
  } v;
} Object;

extern char* TYPE_MAP[];

extern Tree root;
extern Commit commit;

void tree_init();
void build_tree(Tree*, const uint8_t*);
void add_object(Tree*, Object*);
void print_tree(Tree*, int);
void free_tree(Tree*);
void commit_init();
void build_commit(Commit*, const uint8_t*);
void free_commit(Commit*);