#include <dirent.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include "globals.h"
#include "platform.h"
#include "sha256.h"
#include "zstd.h"
#include "object.h"
#include "utils.h"

void crlf_to_lf(char *buffer, size_t *buffer_len) {
  size_t j = 0;
  for (size_t i = 0; i < *buffer_len; i++) {
    if (buffer[i] == '\r' && i + 1 < *buffer_len && buffer[i + 1] == '\n') {
      continue;
    }
    buffer[j++] = buffer[i];
  }
  *buffer_len = j;
}

int is_yagit_repo() {
  DIR *dir;
  struct dirent *entry;
  char path[PATH_MAX];
  bool found = false;
  struct stat st;

  if(GETCWD(path, sizeof(path)) == NULL) {
    perror("Cant fetch cwd: ");
    return -1;
  }
  strncpy(CURRENT_DIR, path, sizeof(CURRENT_DIR));

  while(1) {
    char temp[PATH_MAX];
    snprintf(temp, sizeof(temp), "%s%c", path, PATH_SEP);
    if((dir = opendir(temp)) == NULL) {
      perror("Error opening directory");
      return -1;
    }

    while((entry = readdir(dir)) != NULL) {
      char full_path[PATH_MAX];
      snprintf(full_path, sizeof(full_path), "%s%s", temp, entry->d_name);
      if(stat(full_path, &st) == -1) {
        continue;
      }

      if(S_ISDIR(st.st_mode) && strcmp(entry->d_name, YAGIT_DIR)==0) {
        strncpy(YAGIT_SRC_DIR, path, sizeof(YAGIT_SRC_DIR));
        found = true;
        break;
      }
    }

    if(found) break;

    if(strlen(path) == 2 || strlen(path) == 0) {
      return 0;
    }

    char* p = strrchr(path, PATH_SEP);
    if(p) {
      *p = '\0';
    }
    closedir(dir);
  }

  for(int i=0; i<NO_OF_FOLDERS; i++) {
    for(int j=0; folders[i][j] != NULL; j++) {
      char test[PATH_MAX];
      if(j == 0) {
        build_path(test, 3, YAGIT_SRC_DIR, YAGIT_DIR, folders[i][0]);
      } else {
        build_path(test, 4, YAGIT_SRC_DIR, YAGIT_DIR, folders[i][0], folders[i][j]);
      }

      if(access(test, F_OK) == -1) {
        printf("Where my '%s' at, nah u fucked up, just give up now.\n", folders[i][j]);
      }
    }
  }

  for(int i=1; i<NO_OF_FILES; i++) {
    char test[PATH_MAX];
    build_path(test, 3, path, YAGIT_DIR, files[i]);
    if(access(test, F_OK) == -1) {
      printf("Where my '%s' at, nah u fucked up, just give up now.\n",files[i]);
    }
  }

  return 1;
}

void build_path(char* buffer, int n, ...) {
  va_list args;
  va_start(args, n);

  strncpy(buffer, va_arg(args, const char* ), PATH_MAX);

  for(int i=1; i<n; i++) {
    int index = strlen(buffer);
    if(buffer[index-1] != PATH_SEP)
      buffer[index] = PATH_SEP;
    buffer[++index] = '\0';
    const char *piece = va_arg(args, const char *);
    strncat(buffer, piece, PATH_MAX - strlen(buffer) - 1);
  }
  va_end(args);
}

void write_into_toilet(uint8_t hash[SHA256_DIGEST_SIZE], char *content, size_t size) {
  struct stat st;
  char folder_name[3];
  char file_name[63];
  sprintf(folder_name, "%02x", hash[0]);
  folder_name[2] = '\0';
  for(int i=1; i<SHA256_DIGEST_SIZE; i++) {
    sprintf(&file_name[(i-1)*2], "%02x", hash[i]);
  }
  file_name[62] = '\0';

  char path[PATH_MAX];
  build_path(path, 3, YAGIT_SRC_DIR, YAGIT_DIR, TOILET);
  char folder_path[PATH_MAX];
  snprintf(folder_path, sizeof(folder_path), "%s%c%s", path, PATH_SEP, folder_name);
  char file_path[PATH_MAX];
  snprintf(file_path, sizeof(file_path), "%s%c%s", folder_path, PATH_SEP, file_name);
  if(stat(folder_path, &st) == -1 && MKDIR(folder_path, 0700) == -1) {
    printf("adas");
    return;
  }

  FILE* file = fopen(file_path,"wb");
  if(file == NULL) {
    printf("adas");
    return;
  }

  size_t maxCompressedSize = ZSTD_compressBound(size);
  void* compressed = malloc(maxCompressedSize);
  size_t compressedSize = ZSTD_compress(compressed, maxCompressedSize, content, size, 1);
  if (ZSTD_isError(compressedSize)) {
    fprintf(stderr, "Compression error: %s\n", ZSTD_getErrorName(compressedSize));
    free(compressed);
    return;
  }
  fwrite(compressed, 1, compressedSize, file);
  free(compressed);
  fclose(file);
}

void* read_from_toilet(uint8_t hash[SHA256_DIGEST_SIZE], size_t* out_size) {
    struct stat st;
    char folder_name[3];
    char file_name[63];

    sprintf(folder_name, "%02x", hash[0]);
    folder_name[2] = '\0';
    for (int i = 1; i < SHA256_DIGEST_SIZE; i++) {
        sprintf(&file_name[(i-1)*2], "%02x", hash[i]);
    }
    file_name[62] = '\0';

    char compressed_file_path[PATH_MAX];
    build_path(compressed_file_path, 5, YAGIT_SRC_DIR, YAGIT_DIR, TOILET, folder_name, file_name);

    if (stat(compressed_file_path, &st) == -1) {
      printf("oh no no no");
      return NULL;
    } 

    FILE *compressed_file = fopen(compressed_file_path, "rb");
    if (!compressed_file) return NULL;

    fseek(compressed_file, 0, SEEK_END);
    size_t srcsize = ftell(compressed_file);
    rewind(compressed_file);

    void* compressed = malloc(srcsize);
    if (!compressed) { fclose(compressed_file); return NULL; }

    fread(compressed, 1, srcsize, compressed_file);
    fclose(compressed_file);

    unsigned long long size = ZSTD_getFrameContentSize(compressed, srcsize);
    if (size == ZSTD_CONTENTSIZE_ERROR || size == ZSTD_CONTENTSIZE_UNKNOWN) {
        fprintf(stderr, "Cannot determine decompressed size\n");
        free(compressed);
        return NULL;
    }

    void* decompressed = malloc(size);
    if (!decompressed) {
        fprintf(stderr, "Failed to allocate memory for decompression\n");
        free(compressed);
        return NULL;
    }

    size_t decompressedSize = ZSTD_decompress(decompressed, size, compressed, srcsize);
    free(compressed);

    if (ZSTD_isError(decompressedSize)) {
        fprintf(stderr, "Decompression error: %s\n", ZSTD_getErrorName(decompressedSize));
        free(decompressed);
        return NULL;
    }

    if (out_size) *out_size = decompressedSize;
    return decompressed;
}

void look_at_commit() {
  size_t commit_object_size;
  char* commit_object = read_from_toilet(commit.tree_hash, &commit_object_size);
  fwrite(commit_object, commit_object_size, 1, stdout);
}
