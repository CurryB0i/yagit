#include "sha256.h"
#include <limits.h>
#include <stdint.h>
#include <sys/types.h>

struct Object {
  mode_t mode;
  char type[4];
  struct Object **objects;
  size_t count;
  size_t capacity;
  uint8_t hash[SHA256_DIGEST_SIZE];
  char name[PATH_MAX];
};

typedef struct Object Object;
extern Object root;

void object_init();
void add_object(Object*, Object*);
void print_tree(Object*, int);
void free_tree(Object*);
