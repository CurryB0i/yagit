#include "add.h"
#include "initialize.h"
#include "sha256.h"
#include "globals.h"
#include "zstd.h"
#include "platform.h"
#include <dirent.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int add_file(const char* file_path, const FILE* limbo_file) {
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
  fwrite(blob, 1, blob_len, stdout);

  //hash blob object
  uint8_t sha256_digest[SHA256_DIGEST_SIZE];
  SHA256((const uint8_t*)blob, blob_len, sha256_digest);

  //extract object folder and file name from hash
  char toilet_folder_name[3];
  char toilet_file_name[63];
  sprintf(toilet_folder_name, "%02x", sha256_digest[0]);
  toilet_folder_name[2] = '\0';
  for(int i=0; i<31; i++) {
    sprintf(&toilet_file_name[i*2], "%02x", sha256_digest[i+1]);
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
  strncpy(staged[staged_count++],relative_path, PATH_MAX);

  return 0;
}

int add_folder(const char* folder_path, const FILE* limbo_file) {
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
    snprintf(full_path, sizeof(full_path), "%s%c%s", folder_path, PATH_SEP, entry->d_name);
    if(stat(full_path, &st) == -1) {
      continue;
    }

    if(S_ISDIR(st.st_mode)) {
      add_folder(full_path, limbo_file);
    }

    if(S_ISREG(st.st_mode)) {
      add_file(full_path, limbo_file);
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

  for(int i=2; i<=argc; i++) {
    char entry_path[PATH_MAX];
    snprintf(entry_path, sizeof(entry_path), "%s%c%s", CURRENT_DIR, PATH_SEP, argv[i]);
    if(stat(entry_path, &st) == -1) {
      printf("LOAD OF CRAP!");
      return 1;
    }

    char limbo_path[PATH_MAX];
    build_path(limbo_path, 3, YAGIT_SRC_DIR, YAGIT_DIR, LIMBO);
    FILE* limbo_file = fopen(limbo_path, "w");

    if(S_ISDIR(st.st_mode)) {
      add_folder(entry_path, limbo_file);
    }

    if(S_ISREG(st.st_mode)) {
      add_file(entry_path, limbo_file);
    }

    fclose(limbo_file);
  }
 
  populate_limbo();
  char limbo_file_path[PATH_MAX];
  build_path(limbo_file_path, 3, YAGIT_SRC_DIR, YAGIT_DIR, LIMBO);
  FILE* limbo_file = fopen(limbo_file_path, "w");
  fprintf(limbo_file, "Untracked (%zu)\n", untracked_count);
  for(size_t j=0; j<untracked_count; j++) {
    fprintf(limbo_file, "? %s\n", untracked[j]);
  }
  fprintf(limbo_file, "\nStaged (%zu)\n", staged_count);
  for(size_t j=0; j<staged_count; j++) {
    fprintf(limbo_file, "A %s", staged[j]);
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
