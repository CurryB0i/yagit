#include "commit.h"
#include "globals.h"
#include "limbo.h"
#include "platform.h"
#include "object.h"
#include "sha256.h"
#include "utils.h"
#include "zstd.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <limits.h>
#include <string.h>

Commit commit;

void commit_init() {
  char full_path[PATH_MAX];
  struct stat st;
  build_path(full_path, 5, YAGIT_SRC_DIR, YAGIT_DIR, SNITCHES, HEAD, BRANCH);
  if(stat(full_path, &st) == -1) {
    commit.first = true;
    return;
  } 
  commit.first = false;

  FILE *snitch = fopen(full_path, "r");
  uint8_t commit_hash[SHA256_DIGEST_SIZE];
  size_t decompressed_size;
  fread(commit_hash, SHA256_DIGEST_SIZE, 1, snitch);
  char* decompressed = read_from_toilet(commit_hash, &decompressed_size);
  fclose(snitch);
}

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
  if(!new) {
    printf("adas");
    return NULL;
  }
  new->count = 0;
  strcpy(new->name, path);
  strcpy(new->type, "tree");
  new->mode = st.st_mode;
  new->capacity = 8;
  new->objects = malloc(8 * sizeof(Object*));
  if(!new->objects) {
    printf("adas");
    return NULL;
  }
  add_object(parent, new);
  return new;
}

void calculate_tree_hash(Object* obj) {
  size_t entries_size = (8 + PATH_MAX + SHA256_DIGEST_SIZE) * obj->count;
  char *entries = malloc(entries_size);
  if(!entries) {
    return;
  }
  size_t entries_len = 0;
  
  for(size_t i=0; i<obj->count; i++) {
    if(strcmp(obj->objects[i]->type, "tree") == 0) {
      calculate_tree_hash(obj->objects[i]);
    }
    size_t entry_len = snprintf(entries + entries_len, entries_size - entries_len, "%06o %s", obj->objects[i]->mode, obj->objects[i]->name);
    memcpy(entries + entries_len + entry_len + 1, obj->objects[i]->hash, SHA256_DIGEST_SIZE);
    entries_len += entry_len + SHA256_DIGEST_SIZE + 1;
  }
  
  char header[64];
  int header_len = snprintf(header, sizeof(header), "tree %zu", entries_len);
  size_t buffer_len = header_len + 1 + entries_len;
  char* buffer = malloc(buffer_len);
  if(!buffer) {
    return;
  }
  memcpy(buffer, header, header_len);
  buffer[header_len] = '\0';
  memcpy(buffer + header_len + 1, entries, entries_len);
  SHA256((const uint8_t*)buffer, buffer_len, obj->hash);
  free(entries);
  free(buffer);
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

void make_commit_object() {
  size_t cap = 8192;
  struct stat st;
  size_t offset = 0;
  char* commit_object = malloc(cap);
  if(!commit_object) {
    return;
  }
  strcpy(commit_object, "tree ");
  memcpy(commit_object + 5, root.hash, SHA256_DIGEST_SIZE);
  commit_object[SHA256_DIGEST_SIZE + 6] ='\n';
  offset += SHA256_DIGEST_SIZE + 7;

  if(!commit.first) {
    strcpy(commit_object + offset, "parent ");
    memcpy(commit_object + offset + 7, commit.tree_hash, SHA256_DIGEST_SIZE);
    commit_object[offset + SHA256_DIGEST_SIZE + 7] = '\n';
    offset += SHA256_DIGEST_SIZE + 8;
  }

  offset += snprintf(commit_object + offset, 8192, "author %s %s\ndate", "unknown", "unknown");
  uint8_t commit_hash[SHA256_DIGEST_SIZE];
  SHA256((const uint8_t*)commit_object, offset, commit_hash);
  write_into_toilet(commit_hash, commit_object, offset);
  size_t decompressed;
  read_from_toilet(commit_hash, &decompressed);
  free(commit_object); 
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
    if(!blob) {
      return 1;
    }
    blob->mode = limbo.entries[i].mode;
    memcpy(blob->hash, limbo.entries[i].hash, SHA256_DIGEST_SIZE);
    strcpy(blob->name, file_name);
    blob->objects = NULL;
    strcpy(blob->type, "blob");

    make_tree(temp, blob);
  }

  calculate_tree_hash(&root);
  make_commit_object();
  print_tree(&root, 0);
  free_tree(&root);
  return 0;
}
