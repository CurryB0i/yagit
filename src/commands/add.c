#include "add.h"
#include "SHA256.h"
#include "zstd.h"
#include <stdio.h>
#include <string.h>

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
      for(int i=3;i<argc-1;i++) {
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
  fwrite(blob,1,blob_len,stdout);
  fclose(file);
  return 0;
}
