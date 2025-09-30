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

Tree* find_subtree(Tree* object, const char* path) {
  for(size_t i=0; i<object->count; i++) {
    if(object->objects[i]->type == OBJ_TREE &&
        strcmp(object->objects[i]->v.tree.name, path) == 0) {
      return &(object->objects[i]->v.tree);
    }
  }
  return NULL;
}

Tree* make_tree_object(Tree* parent, const char* path) {
  char full_path[PATH_MAX];
  struct stat st;
  build_path(full_path, 2, YAGIT_SRC_DIR, path);
  stat(full_path, &st);
  Object *new = malloc(sizeof(Object));
  if(!new) {
    printf("adas");
    return NULL;
  }
  new->type = OBJ_TREE;
  new->v.tree.count = 0;
  strcpy(new->v.tree.name, path);
  new->v.tree.mode = st.st_mode;
  new->v.tree.capacity = 8;
  new->v.tree.objects = malloc(8 * sizeof(Object*));
  if(!new->v.tree.objects) {
    printf("adas");
    return NULL;
  }
  add_object(parent, new);
  return &(new->v.tree);
}

void calculate_tree_hash(Tree* obj) {
  size_t entries_size = (8 + PATH_MAX + SHA256_DIGEST_SIZE) * obj->count;
  char *entries = malloc(entries_size);
  if(!entries) {
    return;
  }
  size_t entries_len = 0;

  for(size_t i=0; i<obj->count; i++) {
    if(obj->objects[i]->type == OBJ_TREE) {
      calculate_tree_hash(&(obj->objects[i]->v.tree));
    }
    size_t entry_len = snprintf(entries + entries_len, entries_size - entries_len, "%06o %s",
        obj->objects[i]->v.tree.mode, obj->objects[i]->v.tree.name);
    memcpy(entries + entries_len + entry_len + 1, obj->objects[i]->v.tree.hash, SHA256_DIGEST_SIZE);
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
  write_into_toilet(obj->hash, buffer, buffer_len);
  free(entries);
  free(buffer);
}

void make_tree(const char* path, Object* blob) {
  char* slash;
  Tree* parent;
  Tree* current = &root;
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
  size_t buffer_size = 0;
  char* buffer = malloc(cap);
  if(!buffer) {
    return;
  }
  strcpy(buffer, "tree ");
  memcpy(buffer + 5, root.hash, SHA256_DIGEST_SIZE);
  buffer_size += SHA256_DIGEST_SIZE + 5;

  if(commit.parent_count > 0) {
    strcpy(buffer + buffer_size, "parent ");
    memcpy(buffer + buffer_size + 7, commit.hash, SHA256_DIGEST_SIZE);
    buffer_size += SHA256_DIGEST_SIZE + 7;
  }

  char tz_offset[7];
  snprintf(tz_offset, sizeof(tz_offset), "%+03d%02d", commit.author.tz_offset_minutes / 60, abs(commit.author.tz_offset_minutes % 60));
  buffer_size += sprintf(buffer + buffer_size, "\nauthor %s %s %ld %s\n\n%s", 
      commit.author.name, commit.author.email, commit.author.when, tz_offset, commit.message);

  uint8_t commit_hash[SHA256_DIGEST_SIZE];
  char* commit_object = malloc(cap);
  size_t header_len = snprintf(commit_object, 64, "commit %zu", buffer_size);
  memcpy(commit_object + header_len + 1, buffer, cap - header_len - 1);
  size_t commit_object_size = buffer_size + header_len + 1;
  SHA256((const uint8_t*)commit_object, commit_object_size, commit_hash);
  write_into_toilet(commit_hash, commit_object, commit_object_size);

  char snitch_path[PATH_MAX];
  build_path(snitch_path, 5, YAGIT_SRC_DIR, YAGIT_DIR, SNITCHES, HEADS, BRANCH);
  FILE *snitch_file = fopen(snitch_path, "wb");
  fwrite(commit_hash, 1, SHA256_DIGEST_SIZE, snitch_file);
  
  fclose(snitch_file);
  free(buffer);
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
    blob->type = OBJ_BLOB;
    blob->v.blob.mode = limbo.entries[i].mode;
    memcpy(blob->v.blob.hash, limbo.entries[i].hash, SHA256_DIGEST_SIZE);
    strcpy(blob->v.blob.name, file_name);

    make_tree(temp, blob);
  }

  calculate_tree_hash(&root);
  make_commit_object();
  print_tree(&root, 1);
  free_tree(&root);
  return 0;
}
