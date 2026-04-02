#include <dirent.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "globals.h"
#include "platform.h"
#include "sha256.h"
#include "zstd.h"
#include "object.h"
#include "utils.h"
#include "config.h"

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

void print_error(const char* msg) {
  printf(RED "\n%s\n" RESET, msg);
}

int get_timezone_offset_minutes(time_t now) {
  struct tm gmt = *gmtime(&now);
  struct tm local = *localtime(&now);
  time_t gmt_epoch = mktime(&gmt);
  time_t local_epoch = mktime(&local);
  int offset = (int) difftime(local_epoch, gmt_epoch) / 60;
  return offset;
}

void print_tz_offset(int tz_offset_mins) {
  printf("%+03d%02d", tz_offset_mins / 60, abs(tz_offset_mins % 60));
}

void print_localtime(time_t when) {
  struct tm* local_time_info;
  local_time_info = localtime(&when);

  char time_string[80];
  strftime(time_string, sizeof(time_string), "%a, %b. %d %Y %X", local_time_info);
  /*
    %Y: Year
    %m: Month as a number (01-12)
    %d: Day of the month (01-31)
    %H: Hour in 24-hour format (00-23)
    %M: Minute (00-59)
    %S: Second (00-59)
    %a: Abbreviated weekday name (e.g., Fri)
    %b: Abbreviated month name (e.g., Jul)
    %X: Locale's appropriate time representation (e.g., 10:30:45)
    %c: Locale's appropriate date and time representation
    %Z: Time zone name/abbreviation (if available)
  */
  printf("%s", time_string);
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
      if(STAT(full_path, &st) == -1) {
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
  printf(">");
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
  
  SHA256((const uint8_t*)blob, blob_len, sha256_digest);
  if(blob_out != NULL) {
    *blob_out = blob;
  } else {
    free(blob);
  }

  if(blob_len_out != NULL) {
    *blob_len_out = blob_len;
  }

  return 0;
}

void write_into_toilet(const uint8_t* sha256_digest, char *content, size_t size) {
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
  if(STAT(folder_path, &st) == -1 && MKDIR(folder_path, 0700) == -1) {
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

void* read_from_toilet(const uint8_t* sha256_digest, size_t* out_size) {
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

    if (STAT(compressed_file_path, &st) == -1) {
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

void destruct() {
  free_commit(&commit);
}