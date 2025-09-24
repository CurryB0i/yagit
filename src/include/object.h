#include "sha256.h"
#include <limits.h>
#include <sys/types.h>

struct Object {
  mode_t mode;
  char type[4];
  struct Object **objects;
  size_t count;
  size_t capcaity;
  char hash[SHA256_DIGEST_SIZE];
  char name[PATH_MAX];
};

typedef struct Object Object;

extern Object root;
