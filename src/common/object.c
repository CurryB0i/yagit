#include "object.h"
#include "globals.h"
#include "platform.h"
#include "sha256.h"
#include "utils.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

Tree root;
Commit commit;
char* TYPE_MAP[3] = { "blob", "tree", "commit" };

void tree_init() {
  struct stat st;
  if(stat(YAGIT_SRC_DIR, &st) == -1) return;
  strcpy(root.name, YAGIT_SRC_DIR);
  root.mode = st.st_mode;
  root.object_count = 0;
  root.object_capacity = 8;
  root.objects = malloc(root.object_capacity * sizeof(Object*));
}

void add_object(Tree *parent, Object *child) {
  if(parent->object_count == parent->object_capacity) {
    parent->object_capacity *= 2;
    parent->objects = realloc(parent->objects, parent->object_capacity * sizeof(Object*));
  }
  parent->objects[parent->object_count++] = child;
}

void read_tree_entries(
  Tree* tree,
  const uint8_t* entries,
  size_t entries_length
) {
  size_t offset = 0;
  Mode_t mode;
  uint8_t digest[SHA256_DIGEST_SIZE];
  char name[PATH_MAX];
  char entry_header[64];
  while(offset < entries_length) {
    int entry_header_len = snprintf(entry_header, sizeof(entry_header), "%s", entries + offset);
    sscanf(entry_header, "%06o %s", &mode, name);
    memcpy(digest, entries + offset + entry_header_len + 1, SHA256_DIGEST_SIZE);
    size_t entry_obj_len;
    char *entry_obj = read_from_toilet(digest, &entry_obj_len);

    Object* obj = malloc(sizeof(Object));
    char entry_obj_header[64];
    char obj_type[4];
    size_t obj_content_len;
    int entry_obj_header_len = snprintf(entry_obj_header, sizeof(entry_obj_header), "%s", entry_obj);
    sscanf(entry_obj, "%s %zu", obj_type, &obj_content_len);
    entry_obj += entry_obj_header_len + 1;
    if(strcmp(obj_type, "tree") == 0) {
      obj->type = OBJ_TREE;
      obj->v.tree.mode = mode;
      obj->v.tree.object_count = 0;
      obj->v.tree.object_capacity = 8;
      obj->v.tree.objects = malloc(obj->v.tree.object_capacity * sizeof(Object*));
      strncpy(obj->v.tree.name, name, PATH_MAX);
      memcpy(obj->v.tree.hash, digest, SHA256_DIGEST_SIZE);
      read_tree_entries(&(obj->v.tree), entry_obj, obj_content_len);
    } else if(strcmp(obj_type, "blob") == 0) {
      obj->type = OBJ_BLOB;
      obj->v.blob.mode = mode;
      strncpy(obj->v.blob.name, name, PATH_MAX);
      memcpy(obj->v.blob.hash, digest, SHA256_DIGEST_SIZE);
    } else {
      printf("treason");
      return;
    }
    add_object(tree, obj);
    offset += entry_header_len + SHA256_DIGEST_SIZE + 1;
  }
}

void build_tree(Tree* tree, const uint8_t* tree_hash) {
  memcpy(tree->hash, tree_hash, SHA256_DIGEST_SIZE);

  size_t tree_obj_len;
  char* tree_obj = read_from_toilet(tree_hash, &tree_obj_len);
  if(!tree_obj) {
    printf("Naah bruhhh!");
    destruct();
    exit(1);
  }

  if(strncmp(tree_obj, "tree ", 5) != 0) {
    printf("treason");
    destruct();
    exit(1);
  }

  char tree_obj_header[64];
  size_t tree_obj_header_len = snprintf(tree_obj_header, sizeof(tree_obj_header), "%s", tree_obj);
  size_t tree_obj_content_len;
  sscanf(tree_obj, "tree %zu", &tree_obj_content_len);
  tree_obj += tree_obj_header_len + 1;
  read_tree_entries(tree, tree_obj, tree_obj_content_len);
}

void print_tree(Tree *obj, int depth) {
  printf(BLUE "%s%c\n" RESET, obj->name, PATH_SEP);

  for(size_t i=0; i<obj->object_count; i++) {
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
  for(size_t i=0; i<tree->object_count; i++) {
    if(tree->objects[i]->type == OBJ_TREE) {
      free_tree(&tree->objects[i]->v.tree);
    }
    free(tree->objects[i]);
  }
  tree->object_count = 0;
  tree->object_capacity = 8;
}

void commit_init() {
  char snitch_path[PATH_MAX];
  struct stat st;
  build_path(snitch_path, 5, YAGIT_SRC_DIR, YAGIT_DIR, SNITCHES, HEADS, BRANCH);
  if(stat(snitch_path, &st) == -1) {
    commit.is_first = true;
    return;
  }

  commit.is_first = false;
  strcpy(commit.author.type, "author");
  strcpy(commit.committer.type, "committer");

  FILE *snitch = fopen(snitch_path, "rb");
  uint8_t commit_hash[SHA256_DIGEST_SIZE];
  size_t decompressed_size;
  fread(commit_hash, 1, SHA256_DIGEST_SIZE, snitch);
  build_commit(&commit, commit_hash);
  fclose(snitch);
}

void get_user_data(char* commit_obj, size_t* offset, Identity* ident) {
  if(strncmp(commit_obj + *offset, ident->type, strlen(ident->type)) != 0) {
    printf("treason2");
    destruct();
    exit(1);
  }
  
  *offset += strlen(ident->type) + 1; // space
  int idx = 0;
  while(commit_obj[*offset] != '<') {
    ident->user.name[idx++] = commit_obj[(*offset)++];
  }
  ident->user.name[idx-1] = '\0'; // exclude last space
  (*offset)++;
  
  idx = 0;
  while(commit_obj[*offset] != '>') {
    ident->user.email[idx++] = commit_obj[(*offset)++];
  }
  ident->user.email[idx] = '\0';
  *offset += 2; // skip '>' and ' '

  char when[64];
  idx = 0;
  while(commit_obj[*offset] != ' ') {
    when[idx++] = commit_obj[(*offset)++];
  }
  when[idx] = '\0';
  ident->when = (time_t)atoll(when); 
  (*offset)++;

  int tz_offset_minutes = 0;
  while(commit_obj[*offset] != '\n') {
    tz_offset_minutes = tz_offset_minutes*10 + (commit_obj[(*offset)++] - '0');
  }
  ident->tz_offset_minutes = tz_offset_minutes;
}

void build_commit(Commit* commit, const uint8_t* commit_hash) {
  memcpy(commit->hash, commit_hash, SHA256_DIGEST_SIZE);

  size_t offset = 0;
  size_t commit_obj_len;
  char* commit_obj = read_from_toilet(commit_hash, &commit_obj_len);
  if(!commit_obj) {
    printf("Naah bruhhh!");
    destruct();
    exit(1);
  }

  if(strncmp(commit_obj, "commit ", 7) != 0) {
    printf("treason");
    destruct();
    exit(1);
  }

  char commit_obj_header[64];
  size_t commit_obj_header_len = snprintf(commit_obj_header, sizeof(commit_obj_header), "%s", commit_obj);
  size_t commit_obj_content_len;
  sscanf(commit_obj, "commit %zu", &commit_obj_content_len);
  offset += commit_obj_header_len + 1;

  if(strncmp(commit_obj + offset, "tree ", 5) != 0) {
    printf("treason1");
    exit(1);
  }
  offset += 5;
  memcpy(commit->tree_hash, commit_obj + offset, SHA256_DIGEST_SIZE);
  offset += SHA256_DIGEST_SIZE + 1; // +1 \n

  commit->parents = NULL;
  commit->parent_count = 0;
  while(strncmp(commit_obj + offset, "parent ", 7) == 0) {
    offset += 7;
    commit->parents = realloc(commit->parents, (commit->parent_count + 1) * sizeof(*(commit->parents)));
    memcpy(commit->parents[commit->parent_count++], commit_obj + offset, SHA256_DIGEST_SIZE);
    offset += SHA256_DIGEST_SIZE + 1; // \n
  }

  get_user_data(commit_obj, &offset, &(commit->author));
  offset++; // \n
  get_user_data(commit_obj, &offset, &(commit->committer));
  offset++; // \n
  
  int msg_len = commit_obj_len - offset;
  commit->message = malloc(msg_len + 1);
  for(int idx=0; idx<msg_len; idx++) {
    commit->message[idx] = *(commit_obj + offset++);
  }
  commit->message[msg_len] = '\0';
}

void free_commit(Commit* commit) {
  free(commit->parents);
  free(commit->message);
}