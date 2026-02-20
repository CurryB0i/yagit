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

void print_hash(const uint8_t* sha256_digest) {
  printf("<");
  for(int i=0; i<SHA256_DIGEST_SIZE; i++) {
    printf("%02x", sha256_digest[i]);
  }
  printf(">\n");
}

uint8_t hex_char_to_uint8t(char hex_char) {
  if(hex_char >= '0' && hex_char <= '9') {
    return hex_char - '0';
  } else if(hex_char >= 'a' && hex_char <= 'f') {
    return hex_char - 'a' + 10;
  } else if(hex_char >= 'A' && hex_char <= 'F') {
    return hex_char - 'A' + 10;
  } else {
    printf("Ur Hex string contains blasphemous content! U will be cast to hell ASAP!");
    return 0;
  }
}

int hex_to_uint8t(char* hex_str, size_t hex_str_len, uint8_t (*uint8t_array)[SHA256_DIGEST_SIZE]) {
  if(hex_str_len%2 != 0) {
    printf("I only take hex string hashes! Either ur input is garbage or ur missing a char like ur missing ur dihh!");
    return 1;
  }

  for(int i=0, j=0; i<hex_str_len; i+=2, j++) {
    (*uint8t_array)[j] = ( hex_char_to_uint8t(hex_str[i]) << 4 ) | hex_char_to_uint8t(hex_str[i+1]);
  }

  return 0;
}

int calculate_blob_hash(
  FILE* file, 
  long long file_size, 
  char** blob_out, 
  size_t* blob_len_out, 
  uint8_t* sha256_digest
) {
  char* buffer = malloc(file_size);
  if(!buffer) {
    printf("Something happened and im not going to tell you.");
    return 1;
  }

  size_t buffer_len = fread(buffer, 1, file_size, file);
  crlf_to_lf(buffer, &buffer_len);

  //create yagit blob object
  char header[64];
  int header_len = snprintf(header, sizeof(header), "blob %zu", buffer_len);
  header[header_len] = '\0';
  size_t blob_len = header_len + buffer_len + 1;
  char* blob = malloc(blob_len);
  if(!blob) {
    printf("error");
    free(buffer);
    return 1;
  }

  memcpy(blob, header, header_len);
  blob[header_len] = '\0';
  memcpy(blob + header_len + 1, buffer, buffer_len);
  free(buffer);

  if(blob_out == NULL) {
    free(blob);
  } else {
    *blob_out = blob;
  }

  if(blob_len_out != NULL) {
    *blob_len_out = blob_len;
  }

  return SHA256((const uint8_t*)blob, blob_len, sha256_digest);
}

void write_into_toilet(uint8_t sha256_digest[SHA256_DIGEST_SIZE], char *content, size_t size) {
  struct stat st;
  char folder_name[3];
  char file_name[63];
  sprintf(folder_name, "%02x", sha256_digest[0]);
  folder_name[2] = '\0';
  for(int i=1; i<SHA256_DIGEST_SIZE; i++) {
    sprintf(&file_name[(i-1)*2], "%02x", sha256_digest[i]);
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
  if(!compressed) {
    printf("malloc");
    return;
  }
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

void* read_from_toilet(uint8_t sha256_digest[SHA256_DIGEST_SIZE], size_t* out_size) {
    struct stat st;
    char folder_name[3];
    char file_name[63];

    sprintf(folder_name, "%02x", sha256_digest[0]);
    folder_name[2] = '\0';
    for (int i = 1; i < SHA256_DIGEST_SIZE; i++) {
      sprintf(&file_name[(i-1)*2], "%02x", sha256_digest[i]);
    }
    file_name[62] = '\0';

    char compressed_file_path[PATH_MAX];
    build_path(compressed_file_path, 5, YAGIT_SRC_DIR, YAGIT_DIR, TOILET, folder_name, file_name);

    if (stat(compressed_file_path, &st) == -1) {
      printf("Well Well Well! The file is gone! Or Maybe it never existed! Suck on that!");
      return NULL;
    } 

    FILE *compressed_file = fopen(compressed_file_path, "rb");
    if (!compressed_file) return NULL;

    long long srcsize = (long long)st.st_size;
    void* compressed = malloc(srcsize);
    if (!compressed) { 
      fclose(compressed_file); 
      return NULL; 
    }

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
    crlf_to_lf(decompressed, out_size);
    return decompressed;
}

void read_entry_obj(
  char (*committed)[PATH_MAX],
  size_t* committed_count,
  char* relative_path,
  uint8_t* entry_obj,
  size_t entry_obj_len,
  char* obj_content_out,
  size_t* obj_content_len_out
) {
  char entry_boj_header[64];
  char obj_type[4];
  size_t obj_content_len;
  int entry_obj_header_len = snprintf(entry_boj_header, sizeof(entry_boj_header), "%s", entry_obj);
  sscanf(entry_obj, "%s %zu", obj_type, &obj_content_len);
  entry_obj += entry_obj_header_len + 1;
  if(strcmp(obj_type, "tree") == 0) {
    read_tree_entries(committed, committed_count, relative_path, entry_obj, obj_content_len);
  } else if(strcmp(obj_type, "blob") == 0) {
    if(committed == NULL) {
      if(obj_content_out != NULL && obj_content_len_out != NULL) {
        obj_content_out = entry_obj;
        *obj_content_len_out = obj_content_len;
      } else {
        printf("treason");
        return;
      }
    } else {
      strncpy(committed[(*committed_count)++], relative_path, PATH_MAX);
    }
  } else {
    printf("treason");
    return;
  }
}

void read_tree_entries(
  char (*committed)[PATH_MAX],
  size_t* committed_count,
  char* relative_path,
  const uint8_t* entries,
  size_t entries_length
) {
  size_t offset = 0;
  Mode_t mode;
  char name[PATH_MAX];
  uint8_t digest[SHA256_DIGEST_SIZE];
  while(offset < entries_length) {
    char new_path[PATH_MAX];
    strncpy(new_path, relative_path, PATH_MAX);
    char entry_header[64];
    int entry_header_len = snprintf(entry_header, sizeof(entry_header), "%s", entries + offset);
    sscanf(entry_header, "%06o %s", &mode, name);
    if(strcmp(new_path, "") == 0)
      strncpy(new_path, name, PATH_MAX);
    else
      snprintf(new_path, PATH_MAX, "%s%c%s", new_path, PATH_SEP, name);
    memcpy(digest, entries + offset + entry_header_len + 1, SHA256_DIGEST_SIZE);
    size_t entry_obj_len;
    char *entry_obj = read_from_toilet(digest, &entry_obj_len);
    read_entry_obj(committed, committed_count, new_path, entry_obj, entry_obj_len, NULL, NULL);
    offset += entry_header_len + SHA256_DIGEST_SIZE + 1;
  }
}