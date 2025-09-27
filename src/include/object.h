#include "sha256.h"
#include <limits.h>
#include <stdint.h>
#include <sys/types.h>
#include <time.h>

typedef enum {
  OBJ_BLOB = 1,
  OBJ_TREE = 2,
  OBJ_COMMIT = 3
} ObjectType;

typedef struct {
  char name[256];
  char email[256];
  time_t when;
  int tz_offset_minutes;
} Author;

typedef struct {
  mode_t mode;
  char name[PATH_MAX];
  uint8_t hash[SHA256_DIGEST_SIZE];
} Blob;

typedef struct {
  mode_t mode;
  char name[PATH_MAX];
  uint8_t hash[SHA256_DIGEST_SIZE];
  size_t count;
  size_t capacity;
  struct Object **objects;
} Tree;

typedef struct {
  uint8_t tree_hash[SHA256_DIGEST_SIZE];
  uint8_t (*parent_hash)[SHA256_DIGEST_SIZE];
  size_t parent_count;
  Author author;
  char *message;
} Commit;

typedef struct Object {
  ObjectType type;
  union {
    Blob blob;
    Tree tree;
    Commit commit;
  } v;
} Object;

extern Tree root;
extern Commit commit;

void tree_init();
void commit_init();
void add_object(Tree*, Object*);
void print_tree(Tree*, int);
void free_tree(Tree*);
