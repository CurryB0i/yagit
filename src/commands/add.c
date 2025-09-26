#include <dirent.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "add.h"
#include "utils.h"
#include "sha256.h"
#include "globals.h"
#include "limbo.h"
#include "zstd.h"
#include "platform.h"

uint32_t normalize_mode (mode_t mode) {
  if (S_ISREG(mode)) {
    if (mode & 0111) {
      return 0100755;
    } else {
      return 0100644;
    }
  } else if (S_ISDIR(mode)) {
    return 0040000;
  } else {
    return mode;
  }
}

int add_file(const char* file_path) {
  LimboEntry limbo_entry;
  struct stat st;

  if(stat(file_path, &st) == -1) return 1;
  limbo_entry.mtime_sec = ST_MTIM_SEC(st);
  limbo_entry.mtime_nsec = ST_MTIM_NSEC(st);
  limbo_entry.mode = normalize_mode(st.st_mode);
  limbo_entry.fileSize = st.st_size;

  char relative_path[PATH_MAX];
  strncpy(relative_path, file_path + strlen(YAGIT_SRC_DIR) + 1, sizeof(relative_path));
  relative_path[PATH_MAX-1] = '\0';
  size_t path_len = strlen(relative_path);
  uint16_t flags = 0;
  if (path_len > 0xFFF) {
    flags |= 0xFFF;
  } else {
    flags |= (uint16_t) path_len;
  }
  limbo_entry.flags = flags;
  strcpy(limbo_entry.path,relative_path);

  FILE* file = fopen(file_path,"rb");
  if(file == NULL) {
    printf("This is just a load of crap now.");
    return 1;
  }

  //read content from file
  if(stat(file_path, &st) == -1) {
    printf("adas");
    return 1;
  }

  size_t file_size = st.st_size;

  char* buffer = malloc(file_size);
  if(buffer == NULL) {
    printf("Something happened and im not going to tell you.");
    fclose(file);
    return 1;
  }

  size_t buffer_len = fread(buffer, 1, file_size, file);
  crlf_to_lf(buffer, &buffer_len);
  fclose(file);

  //create yagit blob object
  char header[64];
  int header_len = snprintf(header, sizeof(header), "blob %zu", buffer_len);
  size_t blob_len = header_len + buffer_len + 1;
  char* blob = malloc(blob_len);
  if(!blob) {
    printf("adas");
    return 1;
  }

  memcpy(blob, header, header_len);
  blob[header_len] = '\0';
  memcpy(blob + header_len + 1, buffer, buffer_len);
  free(buffer);

  //hash blob object
  uint8_t sha256_digest[SHA256_DIGEST_SIZE];
  SHA256((const uint8_t*)blob, blob_len, sha256_digest);
  memcpy(limbo_entry.hash, sha256_digest, SHA256_DIGEST_SIZE);
  int added = add_limbo_entry(&limbo_entry);
  if(added) return 0;

  //extract object folder and file name from hash
  write_into_toilet(sha256_digest, blob, blob_len);
  free(blob);
  return 0;
}

int add_folder(const char *folder_path) {
  DIR* dir = opendir(folder_path);
  struct dirent* entry;
  struct stat st;

  if(dir == NULL) {
    printf("adas");
    return 1;
  }

  while((entry = readdir(dir)) != NULL) {
    if(strcmp(entry->d_name,".") == 0 || strcmp(entry->d_name,"..") == 0) {
      continue;
    }
    char full_path[PATH_MAX];
    build_path(full_path, 2, folder_path, entry->d_name);
    if(stat(full_path, &st) == -1) {
      continue;
    }

    if(S_ISDIR(st.st_mode)) {
      add_folder(full_path);
    }

    if(S_ISREG(st.st_mode)) {
      add_file(full_path);
    }
  }

  closedir(dir);
  return 0;
}

int add_command(int argc, char **argv) {
  struct stat st;

  if(argc == 2) {
    printf("add who bitch, yo mama??");
    return 0;
  }

  for(int i=2; i<argc; i++) {
    char entry_path[PATH_MAX];
    snprintf(entry_path, sizeof(entry_path), "%s%c%s", CURRENT_DIR, PATH_SEP, argv[i]);
    if(stat(entry_path, &st) == -1) {
      printf("LOAD OF CRAP!");
      return 1;
    }

    if(S_ISDIR(st.st_mode)) {
      add_folder(entry_path);
    }

    if(S_ISREG(st.st_mode)) {
      add_file(entry_path);
    }
  }

  write_limbo();
  return 0;
}
