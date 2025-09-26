#include "object.h"
#include "globals.h"
#include "platform.h"
#include "sha256.h"
#include "utils.h"
#include <stdio.h>
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
  root.capacity = 8;
  root.objects = malloc(8 * sizeof(Object*));
}

void add_object(Object *parent, Object *child) {
  if(parent->count == parent->capacity) {
    parent->capacity *= 2;
    parent->objects = realloc(parent->objects, parent->capacity * sizeof(Object*));
  }
  parent->objects[parent->count++] = child;
}

void print_tree(Object *obj, int depth) {
  for(size_t i=0; i<obj->count; i++) {
    if(strcmp(obj->objects[i]->type, "blob") == 0) {
      for(size_t j=0; j<depth; j++)
        printf(CYAN "| " RESET);
      printf(GREEN "%s" RESET, obj->objects[i]->name);
      for(size_t j=0; j<SHA256_DIGEST_SIZE; j++) {
        printf("%02x", obj->objects[i]->hash[j]);
      }
      printf("\n");
    }

    if(strcmp(obj->objects[i]->type, "tree") == 0) {
      for(size_t j=0; j<depth; j++)
        printf(CYAN "| " RESET);
      printf(BLUE "%s%c" RESET, obj->objects[i]->name, PATH_SEP);
      for(size_t j=0; j<SHA256_DIGEST_SIZE; j++) {
        printf("%02x", obj->objects[i]->hash[j]);
      }
      printf("\n");
      print_tree(obj->objects[i], depth+1);
    }

  }
}

void free_tree(Object *obj) {
  for (size_t i = 0; i < obj->count; i++) {
    if (strcmp(obj->objects[i]->type, "tree") == 0) {
      free_tree(obj->objects[i]);
      free(obj->objects[i]);
    } else {
      free(obj->objects[i]);
    }
  }
}
