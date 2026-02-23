#include "globals.h"
#include "cat-file.h"
#include "SHA256.h"
#include "utils.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int cat_file_command(int argc, char* argv[]) {
  if(argc == 2) {
    print_error("Gimme the hash u pussy aah!");
    return 0;
  }

  uint8_t (*sha256_digests)[SHA256_DIGEST_SIZE] = malloc((argc-2) * SHA256_DIGEST_SIZE);
  size_t digest_count = 0;
  for(int i=2; i<argc; i++) {
    if(strlen(argv[i]) != SHA256_DIGEST_SIZE*2) {
      print_error("Ur hash(es) looks funny, who u tryna fool here bitch ass!");
      return 0;
    }

    int status = hex_to_uint8t(argv[i], SHA256_DIGEST_SIZE*2, &sha256_digests[0]);
    if(status != 0) {
      print_error("U know something's wrong and thought i wouldnt notice!");
      return 1;
    }

    size_t object_len;
    char* object = read_from_toilet(sha256_digests[0], &object_len);
    fwrite(object, object_len, 1, stdout);
  }
}