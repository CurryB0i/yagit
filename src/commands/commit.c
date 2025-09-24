#include "commit.h"
#include "globals.h"
#include "limbo.h"
#include "platform.h"
#include "object.h"
#include "sha256.h"
#include <sys/stat.h>
#include <limits.h>
#include <string.h>

bool get_subdir(const char* path) {
  
}

Object* find_subtree(Object* object, const char* path) {
  for(size_t i=0; i<object->count; i++) {
    if(strcmp(object->objects[i]->name, path) == 0) 
      return object->objects[i];
  }
}

void make_tree_object(const char* path, const Object* object) {
  char* last_slash;
  struct stat st;
  char full_path[PATH_MAX];
  while((last_slash = strrchr(path, PATH_SEP)) != NULL) {
    build_path(full_path, 2, YAGIT_SRC_DIR, path);
    if(stat(full_path, &st) == -1) continue;
    Object *tree;
    tree = find_subtree(root, path);
    char* name = last_slash + 1;

  }

  Object tree;
  

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

    Object blob;
    blob.mode = limbo.entries[i].mode;
    memcpy(blob.hash, limbo.entries[i].hash, SHA256_DIGEST_SIZE);
    strcpy(blob.name, file_name);
    blob.objects = NULL;
    strcpy(blob.type, "blob");

    if(last_slash != NULL)
      make_tree_object(temp, &blob);
    
  } 

  return 0;
}
