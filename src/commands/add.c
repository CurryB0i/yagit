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
#include "zstd.h"
#include "platform.h"

int add_file(const char* file_path, FILE* limbo_file, bool lazy_recurse) {
  FILE* file = fopen(file_path,"rb");
  struct stat st;

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

  memcpy(blob, header, header_len);
  blob[header_len] = '\0';
  memcpy(blob + header_len + 1, buffer, buffer_len);
  free(buffer);

  //hash blob object
  uint8_t sha256_digest[SHA256_DIGEST_SIZE];
  SHA256((const uint8_t*)blob, blob_len, sha256_digest);

  //extract object folder and file name from hash
  char toilet_folder_name[3];
  char toilet_file_name[63];
  sprintf(toilet_folder_name, "%02x", sha256_digest[0]);
  toilet_folder_name[2] = '\0';
  for(int i=1; i<SHA256_DIGEST_SIZE; i++) {
    sprintf(&toilet_file_name[(i-1)*2], "%02x", sha256_digest[i+1]);
  }
  toilet_file_name[62] = '\0';

  //create folder and file
  char toilet_path[PATH_MAX];
  build_path(toilet_path, 3, YAGIT_SRC_DIR, YAGIT_DIR, TOILET);
  char toilet_folder_path[PATH_MAX];
  snprintf(toilet_folder_path, sizeof(toilet_folder_path), "%s%c%s", toilet_path, PATH_SEP, toilet_folder_name);
  char toilet_file_path[PATH_MAX];
  snprintf(toilet_file_path, sizeof(toilet_file_path), "%s%c%s", toilet_folder_path, PATH_SEP, toilet_file_name);
  if(stat(toilet_folder_path, &st) == -1 && MKDIR(toilet_folder_path, 0700) == -1) {
    printf("adas");
    return 1;
  }

  FILE* toilet_file;
  toilet_file = fopen(toilet_file_path,"w");
  if(toilet_file == NULL) {
    printf("adas");
    return 1;
  }

  //compress content
  size_t maxCompressedSize = ZSTD_compressBound(blob_len);
  void* compressed = malloc(maxCompressedSize);

  size_t compressedSize = ZSTD_compress(compressed, maxCompressedSize, blob, blob_len, 1);
  if (ZSTD_isError(compressedSize)) {
    fprintf(stderr, "Compression error: %s\n", ZSTD_getErrorName(compressedSize));
    free(compressed);
    free(blob);
    return 1;
  }
  free(blob);

  //write compressed content into object file
  fwrite(compressed, compressedSize, 1, toilet_file);
  free(compressed);
  fclose(toilet_file);

  char relative_path[PATH_MAX];
  strncpy(relative_path, file_path + strlen(YAGIT_SRC_DIR) + 1, sizeof(relative_path));
  relative_path[PATH_MAX-1] = '\0';
  for(size_t i=0; i<staged_count; i++) {
    if(strcmp(staged[i], relative_path) == 0)
      return 0;
  }

  for(size_t i=0; i<untracked_count; i++) {
    if(strcmp(untracked[i], relative_path) == 0) {
      for(size_t j=i; j<untracked_count; j++) {
        strncpy(untracked[j], untracked[j+1], PATH_MAX);
      }
      untracked_count--;
    }
  }

  //lazy recurse directories
  if(lazy_recurse) {
    char *temp = malloc(PATH_MAX);
    if(!temp) {
      printf("Adas");
      return 1;
    }
    strncpy(temp, relative_path, PATH_MAX);
    char* slash;
    while((slash = strrchr(temp, PATH_SEP)) != NULL) {
      *slash = '\0';
      char temp2[PATH_MAX];
      snprintf(temp2, sizeof(temp2), "%s%c", temp, PATH_SEP);
      bool flag = false;
      for(size_t i=0; i<untracked_count; i++) {
        if(strcmp(untracked[i], temp2) == 0) {
          char full_path[PATH_MAX];
          build_path(full_path, 2, YAGIT_SRC_DIR, untracked[i]);
          populate_dir_entries(full_path);
          for(size_t j=i; j<untracked_count-1; j++) {
            strncpy(untracked[j], untracked[j+1], PATH_MAX);
          }
          untracked_count--;
          i--;
          flag = true;
        }
      }
      if(!flag) {
        char full_path[PATH_MAX];
        build_path(full_path, 2, YAGIT_SRC_DIR, temp2);
        populate_dir_entries(full_path);
      }
    }
    free(temp);
  }

  if(staged_count == staged_cap) {
    staged_cap *= 2;
    staged = realloc(staged, staged_cap * sizeof(*staged));
  }
  strncpy(staged[staged_count++],relative_path, PATH_MAX);
  return 0;
}

int add_folder(const char *folder_path, FILE *limbo_file) {
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
      add_folder(full_path, limbo_file);
    }

    if(S_ISREG(st.st_mode)) {
      add_file(full_path, limbo_file, false);
    }
  }

  return 0;
}

int add_command(int argc, char **argv) {
  DIR* dir;
  struct dirent* entry;
  struct stat st;

  if(argc == 1) {
    printf("add who bitch, yo mama??");
    return 0;
  }

  char limbo_path[PATH_MAX];
  build_path(limbo_path, 3, YAGIT_SRC_DIR, YAGIT_DIR, LIMBO);
  populate_limbo();
  FILE* limbo_file = fopen(limbo_path, "wb");

  for(int i=2; i<=argc; i++) {
    char entry_path[PATH_MAX];
    snprintf(entry_path, sizeof(entry_path), "%s%c%s", CURRENT_DIR, PATH_SEP, argv[i]);
    if(stat(entry_path, &st) == -1) {
      printf("LOAD OF CRAP!");
      return 1;
    }

    if(S_ISDIR(st.st_mode)) {
      add_folder(entry_path, limbo_file);
      char relative_path[PATH_MAX];
      snprintf(relative_path, sizeof(relative_path), "%s%c", entry_path + strlen(YAGIT_SRC_DIR) + 1, PATH_SEP);
      relative_path[PATH_MAX-1] = '\0';
      for(size_t j=0; j<untracked_count; j++) {
        if(strcmp(untracked[j], relative_path) == 0) {
          for(size_t k=j; k<untracked_count-1; k++) {
            strncpy(untracked[k], untracked[k+1], PATH_MAX);
          }
          untracked_count--;
        }
      }
    }

    if(S_ISREG(st.st_mode)) {
      add_file(entry_path, limbo_file, true);
    }
  }

 
  fprintf(limbo_file, "Untracked (%zu)\n", untracked_count);
  for(size_t j=0; j<untracked_count; j++) {
    fprintf(limbo_file, "? %s\n", untracked[j]);
  }
  fprintf(limbo_file, "\nStaged (%zu)\n", staged_count);
  for(size_t j=0; j<staged_count; j++) {
    fprintf(limbo_file, "A %s\n", staged[j]);
  }
  fclose(limbo_file);
  return 0;
}

/*
void* decompressed = malloc(blob_len);
if (!decompressed) {
    fprintf(stderr, "Failed to allocate memory for decompression\n");
    free(compressed);
    return 1;
}
size_t decompressedSize = ZSTD_decompress(decompressed, blob_len, compressed, compressedSize);
if (ZSTD_isError(decompressedSize)) {
  fprintf(stderr, "Decompression error: %s\n", ZSTD_getErrorName(decompressedSize));
  free(compressed);
  free(decompressed);
  return 1;
}
*/
