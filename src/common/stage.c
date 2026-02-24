#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>
#include <dirent.h>
#include "platform.h"
#include "globals.h"
#include "object.h"
#include "utils.h"
#include "limbo.h"

size_t staged_count = 0;
size_t unstaged_count = 0;
size_t committed_count = 0;
size_t untracked_count = 0;
size_t visited_count = 0;
size_t untracked_cap = 8;
char (*staged)[PATH_MAX];
char (*unstaged)[PATH_MAX];
char (*committed)[PATH_MAX];
char (*untracked)[PATH_MAX];

bool is_modified(LimboEntry *entry) {
  struct stat st;
  char file_path[PATH_MAX];
  build_path(file_path, 2, YAGIT_SRC_DIR, entry->path);
  if(STAT(file_path, &st) == -1) {
    printf("poof magic");
    return true;
  }

  uint64_t calc_mtime = ((uint64_t) ST_MTIM_SEC(st) << 32) | ST_MTIM_NSEC(st);
  uint64_t act_mtime = ((uint64_t) entry->mtime_sec << 32 ) | entry->mtime_nsec;
  if((st.st_size != entry->fileSize) || 
     (calc_mtime != act_mtime)) {

    FILE* file = fopen(file_path, "rb");
    long long file_size = (long long)st.st_size;
    if(file == NULL) {
      printf("error");
      return false;
    }

    uint8_t sha256_digest[SHA256_DIGEST_SIZE];
    int status = calculate_blob_hash(file, file_size, NULL, NULL, sha256_digest);
    fclose(file);
    if(status != 0) {
      printf("error");
      return true;
    }

    if(memcmp(sha256_digest, entry->hash, SHA256_DIGEST_SIZE) == 0) {
      return false;
    } else {
      return true;
    }
  }
  return false;
}

bool visited(const char* dir_path) {
  for(size_t i=0; i<staged_count; i++) {
    if(strncmp(staged[i], dir_path, strlen(dir_path)) == 0) 
      return true;
  }

  for(size_t i=0; i<unstaged_count; i++) {
    if(strncmp(unstaged[i], dir_path, strlen(dir_path)) == 0) 
      return true;
  } 

  return false;
}

bool recursively_empty(const char* dir_path) {
  DIR *dir = opendir(dir_path);
  if(dir == NULL) return true;
  struct dirent* entry;
  struct stat st;

  while((entry = readdir(dir)) != NULL) {
    if((entry->d_name[0] == '.' && entry->d_name[1] == '\0') ||
        entry->d_name[0] == '.' && entry->d_name[1] == '.' && entry->d_name[2] == '\0') {
      continue;
    }

    char entry_path[PATH_MAX];
    build_path(entry_path, 2, dir_path, entry->d_name);
    if(STAT(entry_path, &st) == -1) return true;

    if(S_ISDIR(st.st_mode)) {
      return recursively_empty(entry_path);
    }

    if(S_ISREG(st.st_mode)) {
      return false;
    }
  }

  return true;
}

bool tracked(const char* entry) {
  for(size_t i=0; i<staged_count; i++) {
    if(strcmp(staged[i], entry) == 0)
      return true;
  }

  for(size_t i=0; i<unstaged_count; i++) {
    if(strcmp(unstaged[i], entry) == 0)
      return true;
  }

  return false;
}

void walk_dir(const char* path) {
  DIR *dir;
  struct dirent *entry;
  struct stat st;
  if((dir = opendir(path)) == NULL) return;
  while((entry = readdir(dir)) != NULL) {
    if((entry->d_name[0] == '.' && entry->d_name[1] == '\0') ||
        (entry->d_name[0] == '.' && entry->d_name[1] == '.' && entry->d_name[2] == '\0') ||
        strcmp(entry->d_name, YAGIT_DIR) == 0) {
      continue;
    }

    char entry_path[PATH_MAX];
    build_path(entry_path, 2, path, entry->d_name);
    if(STAT(entry_path, &st) == -1) continue;
    if(S_ISDIR(st.st_mode)) {
      if(recursively_empty(entry_path))
        continue;

      if(visited(entry_path + strlen(YAGIT_SRC_DIR) + 1)) {
        walk_dir(entry_path);
      } else {
        if(untracked_cap == untracked_count) {
          untracked_cap *= 2;
          untracked = realloc(untracked, untracked_cap * PATH_MAX);
        }
        snprintf(untracked[untracked_count++], PATH_MAX, "%s%c", entry_path + strlen(YAGIT_SRC_DIR) + 1, PATH_SEP);
      }
    }

    if(S_ISREG(st.st_mode)) {
      if(tracked(entry_path + strlen(YAGIT_SRC_DIR) + 1))
        continue;

      if(untracked_cap == untracked_count) {
        untracked_cap *= 2;
        untracked = realloc(untracked, untracked_cap * PATH_MAX);
      }
      strncpy(untracked[untracked_count++], entry_path + strlen(YAGIT_SRC_DIR) + 1, PATH_MAX);
    }
  }
}

bool is_committed(Tree* tree, char* path, const uint8_t* hash) {
  char* slash;
  slash = strchr(path, PATH_SEP);
  if(slash == NULL) {
    for(int i=0; i<tree->object_count; i++) {
      if(tree->objects[i]->type != OBJ_BLOB)
        continue;

      if(strcmp(path, tree->objects[i]->v.blob.name) == 0) {
        return (memcmp(tree->objects[i]->v.blob.hash, hash, SHA256_DIGEST_SIZE) == 0);
      }
    }
    return 0;
  } else {
    *slash = '\0';
    for(int i=0; i<tree->object_count; i++) {
      if(tree->objects[i]->type != OBJ_TREE)
        continue;

      if(strcmp(path, tree->objects[i]->v.tree.name) == 0) {
        path = slash + 1;
        return is_committed(&(tree->objects[i]->v.tree), path, hash);
      }
    }
    return 0;
  }
}

void diff_against_head() {
  if(commit.is_first) return;

  size_t commit_object_size;
  char* commit_object = read_from_toilet(commit.tree_hash, &commit_object_size);
  if(commit_object == NULL) return;

  char header[64];
  size_t entries_len;
  int header_len = snprintf(header, sizeof(header), "%s", commit_object);
  int status = sscanf(header, "tree %zu", &entries_len);
  if(status != 1) {
    printf("ERROR");
    exit(0);
  }
  char entries[entries_len];
  memcpy(entries, commit_object + header_len + 1, entries_len);
  char relative_path[PATH_MAX] = "";
  build_tree(&root, commit.tree_hash);

  for(int i=0; i<limbo.header.entry_count; i++) {
    char path[PATH_MAX];
    strncpy(path, limbo.entries[i].path, PATH_MAX);
    if(is_committed(&root, path, limbo.entries[i].hash)) {
      strncpy(committed[committed_count++], limbo.entries[i].path, PATH_MAX);
    }
  }

  for(int i=0; i<committed_count; i++) {
    for(int j=0; j<staged_count; j++) {
      if(strncmp(committed[i], staged[j], PATH_MAX) == 0) {
        for(int k=j; k<staged_count-1; k++) {
          char temp[PATH_MAX];
          strncpy(temp, staged[k], PATH_MAX);
          strncpy(staged[k], staged[k+1], PATH_MAX);
          strncpy(staged[k+1], temp, PATH_MAX);
        }
        --staged_count;
      }
    }
  }
}

void set_stage() {
  staged_count = 0;
  unstaged_count = 0;
  committed_count = 0;
  untracked_count = 0;
  staged = malloc(limbo.header.entry_count * sizeof(*staged));
  unstaged = malloc(limbo.header.entry_count * sizeof(*unstaged));
  committed = malloc(limbo.header.entry_count * sizeof(*committed));
  untracked = malloc(untracked_cap * sizeof(*untracked));

  for(size_t i=0; i<limbo.header.entry_count; i++) {
    if(is_modified(&limbo.entries[i])) {
      strncpy(unstaged[unstaged_count++], limbo.entries[i].path, PATH_MAX);
    } else {
      strncpy(staged[staged_count++], limbo.entries[i].path, PATH_MAX);
    }
  }

  walk_dir(YAGIT_SRC_DIR);
  diff_against_head();
}

void destroy_stage() {
    free(staged);
    free(unstaged);
    free(committed);
    free(untracked);
}