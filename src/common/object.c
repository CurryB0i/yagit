#include "object.h"
#include "globals.h"
#include "platform.h"
#include "sha256.h"
#include "utils.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

Tree root;
Commit commit;

void tree_init() {
  struct stat st;
  if(stat(YAGIT_SRC_DIR, &st) == -1) return;
  strcpy(root.name, YAGIT_SRC_DIR);
  root.mode = st.st_mode;
  root.count = 0;
  root.capacity = 8;
  root.objects = malloc(8 * sizeof(Object*));
}

void add_object(Tree *parent, Object *child) {
  if(parent->count == parent->capacity) {
    parent->capacity *= 2;
    parent->objects = realloc(parent->objects, parent->capacity * sizeof(Object*));
  }
  parent->objects[parent->count++] = child;
}

int get_timezone_offset_minutes(time_t now) {
  struct tm gmt = *gmtime(&now);
  struct tm local = *localtime(&now);
  time_t gmt_epoch = mktime(&gmt);
  time_t local_epoch = mktime(&local);
  int offset = (int) difftime(local_epoch, gmt_epoch) / 60;
  return offset;
}

void commit_init() {
  strcpy(commit.author.name, "unknown");
  strcpy(commit.author.email, "unknown");
  commit.author.when = time(NULL);
  commit.author.tz_offset_minutes = get_timezone_offset_minutes(commit.author.when);
  commit.message = malloc(sizeof("initial commit pooki man"));
  strcpy(commit.message, "initial commit pooki man");
  char snitch_path[PATH_MAX];
  struct stat st;
  build_path(snitch_path, 5, YAGIT_SRC_DIR, YAGIT_DIR, SNITCHES, HEADS, BRANCH);
  commit.parent_count = 0;
  if(stat(snitch_path, &st) == -1) {
    commit.parent_hash = NULL;
    return;
  }

  FILE *snitch = fopen(snitch_path, "rb");
  uint8_t commit_hash[SHA256_DIGEST_SIZE];
  size_t decompressed_size;
  fread(commit_hash, 1, SHA256_DIGEST_SIZE, snitch);
  memcpy(commit.hash, commit_hash, SHA256_DIGEST_SIZE);
  void* decompressed = read_from_toilet(commit_hash, &decompressed_size);
  if(!decompressed) {
    return;
  }
  memcpy(commit.tree_hash, decompressed + 8, SHA256_DIGEST_SIZE);
  int offset = 8;
  decompressed += offset + SHA256_DIGEST_SIZE;
  while(strncmp(decompressed + offset, "parent ", 6) == 0) {
    offset += 6;
    commit.parent_hash = realloc(commit.parent_hash, ++commit.parent_count * SHA256_DIGEST_SIZE);
    memcpy(commit.parent_hash[commit.parent_count], decompressed + offset, SHA256_DIGEST_SIZE);
    offset += SHA256_DIGEST_SIZE;
  }
  
  fclose(snitch);
}

void print_tree(Tree *obj, int depth) {
  printf(BLUE "%s%c\n" RESET, obj->name, PATH_SEP);

  for(size_t i=0; i<obj->count; i++) {
    Object *curr = obj->objects[i];
    if(curr->type == OBJ_BLOB) {
      for(size_t j=0; j<depth; j++)
        printf(CYAN "| " RESET);
      printf(GREEN "%s\n" RESET, curr->v.blob.name);
    }

    if(curr->type == OBJ_TREE) {
      for(size_t j=0; j<depth; j++)
        printf(CYAN "| " RESET);
      print_tree(&(curr->v.tree), depth+1);
    }

  }
}

void free_tree(Tree* tree) {
  for(size_t i=0; i<tree->count; i++) {
    if(tree->objects[i]->type == OBJ_TREE) {
      free_tree(&tree->objects[i]->v.tree);
    }
    free(tree->objects[i]);
  }
}
