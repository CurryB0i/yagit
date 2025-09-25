#include "commit.h"
#include "globals.h"
#include "limbo.h"
#include "platform.h"
#include "object.h"
#include "sha256.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <limits.h>
#include <string.h>

Object* find_subtree(Object* object, const char* path) {
  for(size_t i=0; i<object->count; i++) {
    if(strcmp(object->objects[i]->name, path) == 0)  {
      return object->objects[i];
    }
  }
  return NULL;
}

Object* make_tree_object(Object* parent, const char* path) {
  char full_path[PATH_MAX];
  struct stat st;
  build_path(full_path, 2, YAGIT_SRC_DIR, path);
  stat(full_path, &st);
  Object *new = malloc(sizeof(Object));
  new->count = 0;
  strcpy(new->name, path);
  strcpy(new->type, "tree");
  new->mode = st.st_mode;
  new->capacity = 8;
  new->objects = malloc(8 * sizeof(Object*));
  add_object(parent, new);
  return new;
}

void calculate_hash(Object* obj) {
  char *buffer = malloc(300 * obj->count);
  int buffer_len = 0;
  for(size_t i=0; i<obj->count; i++) {
    if(strcmp(obj->objects[i]->type, "tree") == 0) {
      calculate_hash(obj->objects[i]);
    } else {
      int len = snprintf(buffer + buffer_len, 300 * obj->count, "%u %s", obj->objects[i]->mode, obj->objects[i]->name);
      buffer[len] = '\0';
      strncpy(buffer + len + 1, obj->objects[i]->hash, SHA256_DIGEST_SIZE);
      buffer_len += len + SHA256_DIGEST_SIZE + 1;
    }
  }
  SHA256(buffer, buffer_len, obj->hash);
}

void make_tree(const char* path, Object* blob) {
  char* slash;
  Object* parent;
  Object* current = &root;
  while((slash = strchr(path, PATH_SEP)) != NULL) {
    *slash = '\0';
    parent = current;
    current = find_subtree(parent, path);
    if(current == NULL) {
      current = make_tree_object(parent, path);
    }
    *slash = PATH_SEP;
    path = slash + 1;
  }

  add_object(current, blob);
}

int commit_command(int argc, char* argv[]) {
  for(size_t i=0; i<limbo.header.entry_count; i++) {
    char temp[PATH_MAX];
    strcpy(temp, limbo.entries[i].path);
    char* last_slash = strrchr(temp, PATH_SEP);
    char* file_name;
    if(last_slash != NULL)
      file_name = last_slash + 1;
    else
      file_name = limbo.entries[i].path;

    Object *blob = malloc(sizeof(Object));
    blob->mode = limbo.entries[i].mode;
    memcpy(blob->hash, limbo.entries[i].hash, SHA256_DIGEST_SIZE);
    strcpy(blob->name, file_name);
    blob->objects = NULL;
    strcpy(blob->type, "blob");

    make_tree(temp, blob);
    calculate_hash(&root);
  }

  print_tree(&root, 0);

  return 0;
}
