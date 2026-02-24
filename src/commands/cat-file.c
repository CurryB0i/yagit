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

    int status = hex_to_uint8t(argv[i], SHA256_DIGEST_SIZE*2, &sha256_digests[i-2]);
    if(status != 0) {
      print_error("U know something's wrong and thought i wouldnt notice!");
      return 1;
    }

    size_t object_len;
    char* object = read_from_toilet(sha256_digests[i-2], &object_len);
    char header[64];
    char type[10];
    size_t obj_content_len;
    int header_len = snprintf(header, sizeof(header), "%s", object);
    sscanf(header, "%s %zu", type, &obj_content_len);
    if(strncmp(type, "commit", 6) == 0) {
      Commit c;
      build_commit(&c, sha256_digests[i-2]);

      printf(GREEN "\ntree ");
      print_hash(c.tree_hash);
      printf(RESET);

      for(int j=0; j<c.parent_count; j++) {
        printf(BLUE "\nparent ");
        print_hash(c.parents[j]);
      }
      printf(RESET);

      printf(MAGENTA "\nAuthor: %s %s %lld ", c.author.user.name, c.author.user.email, c.author.when);
      print_tz_offset(c.author.tz_offset_minutes);

      printf("\nCommitter: %s %s %lld ", c.committer.user.name, c.committer.user.email, c.committer.when);
      print_tz_offset(c.committer.tz_offset_minutes);

      printf(CYAN "\n\n%s\n" RESET ,c.message);

    } else if (strncmp(type, "tree", 4) == 0) {
      Tree t;
      build_tree(&t, sha256_digests[i-2]);
      printf("\n");
      for(int j=0; j<t.object_count; j++) {
        if(t.objects[j]->type == OBJ_TREE) {
          printf(GREEN "%06o tree ", t.objects[j]->v.tree.mode);
          print_hash(t.objects[j]->v.tree.hash);
          printf(" %s\n" RESET, t.objects[j]->v.tree.name);
        } else {
          printf(YELLOW "%06o blob ", t.objects[j]->v.blob.mode);
          print_hash(t.objects[j]->v.blob.hash);
          printf(" %s\n" RESET, t.objects[j]->v.blob.name);
        }
      }
    } else if(strncmp(type, "blob", 4) == 0) {
      printf("\n");
      fwrite(object + header_len + 1, 1, obj_content_len, stdout);
      printf("\n");
    } else {
      print_error("\nTampering with the toiler. TREASON!\n");
    }

  }
}