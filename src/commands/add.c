#include "add.h"
#include "SHA256.h"
#include "globals.h"
#include "zstd.h"
#include "platform.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>

int compress_content(void* compressed,const char* content,size_t content_len) {
}

int add_command(int argc, char **argv) {
 /*   const char* input = "Hello, Zstandard compression!";
    size_t inputSize = strlen(input) + 1; // +1 to include null terminator

    size_t maxCompressedSize = ZSTD_compressBound(inputSize);
    void* compressed = malloc(maxCompressedSize);
    if (!compressed) {
        fprintf(stderr, "Failed to allocate memory for compression\n");
        return 1;
    }

    size_t compressedSize = ZSTD_compress(compressed, maxCompressedSize, input, inputSize, 1);
    if (ZSTD_isError(compressedSize)) {
        fprintf(stderr, "Compression error: %s\n", ZSTD_getErrorName(compressedSize));
        free(compressed);
        return 1;
    }

    printf("Original size: %zu bytes\n", inputSize);
    printf("Compressed size: %zu bytes\n", compressedSize);

    void* decompressed = malloc(inputSize);
    if (!decompressed) {
        fprintf(stderr, "Failed to allocate memory for decompression\n");
        free(compressed);
        return 1;
    }

    size_t decompressedSize = ZSTD_decompress(decompressed, inputSize, compressed, compressedSize);
    if (ZSTD_isError(decompressedSize)) {
        fprintf(stderr, "Decompression error: %s\n", ZSTD_getErrorName(decompressedSize));
        free(compressed);
        free(decompressed);
        return 1;
    }

    printf("Decompressed size: %zu bytes\n", decompressedSize);
    printf("Decompressed text: %s\n", (char*)decompressed);

    free(compressed);
    free(decompressed);

    return 0; */
  if(argc == 1) {
    printf("add who bitch, yo mama??");
    return 0;
  }

  if(argc > 2) {
    printf("who tf ");
    if(argc == 3) {
      printf("is '%s' ",argv[3]);
    } else {
      printf("are ");
      for(int i=3;i<argc;i++) {
        printf("'%s', ",argv[i]);
      }
      printf("and '%s' ",argv[argc]);
    }
    printf("dickweed??");
    return 0;
  }

  FILE* file = fopen(argv[2],"r");
  if(file == NULL) {
    printf("This is just a load of crap now.");
    return 1;
  }

  fseek(file,0,SEEK_END);
  size_t size = ftell(file);
  rewind(file);

  char *buffer = malloc(size+1);
  if(!buffer) {
    printf("Something happened and im not going to tell you.");
    fclose(file);
    return 1;
  }

  size_t nread = fread(buffer,1,size,file);
  buffer[nread] = '\0';

  char header[64];
  int header_len = snprintf(header, sizeof(header), "blob %zu", size);
  size_t blob_len = header_len + 1 + size;
  char* blob = malloc(blob_len);
  memcpy(blob, header, header_len);
  blob[header_len] = '\0';
  memcpy(blob + header_len + 1, buffer, size);

  char object_id[64];
  SHA256(blob,object_id);
  char folder_name[3],file_name[63];
  strncpy(folder_name,object_id,2);
  strncpy(file_name,object_id+2,62);
  folder_name[2] = '\0';
  file_name[62] = '\0';
  char path[PATH_MAX];
  build_path(path,sizeof(path),TOILET);
  printf("%s",YAGIT_SRC_DIR);
  snprintf(path,PATH_MAX,"%s\\%s",path,folder_name);
  if(MKDIR(path, 0700) == -1) {
    printf("gay gay gay");
  }
  strncpy(path+2,file_name,sizeof(file_name));
  FILE* blob_file = fopen(path,"w");
  size_t maxCompressedSize = ZSTD_compressBound(blob_len+1);
  void* compressed = malloc(maxCompressedSize);
  if (!compressed) {
    fprintf(stderr, "Failed to allocate memory for compression\n");
    return 1;
  }

  size_t compressedSize = ZSTD_compress(compressed, maxCompressedSize, blob, blob_len+1, 1);
  if (ZSTD_isError(compressedSize)) {
    fprintf(stderr, "Compression error: %s\n", ZSTD_getErrorName(compressedSize));
    free(compressed);
    return 1;
  }

  fwrite(compressed,1,compressedSize,blob_file);

  fclose(file);
  return 0;
}
