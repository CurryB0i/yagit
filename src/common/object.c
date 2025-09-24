#include "object.h"
#include "globals.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

Object root;

void object_init() {
  struct stat st;

  if(stat(YAGIT_SRC_DIR, &st) == -1) return;
  strcpy(root.type, "tree");
  strcpy(root.name, YAGIT_SRC_DIR);
  root.mode = st.st_mode;
  root.count = 0;
  root.capcaity = 8;
  root.objects = malloc(root.capcaity * sizeof(Object*));
}
