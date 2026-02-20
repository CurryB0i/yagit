#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "limbo.h"
#include "globals.h"
#include "utils.h"
#include "platform.h"
#include "sha256.h"

Limbo limbo;

void limbo_init() {
  strcpy(limbo.header.magic, "LMBO");
  limbo.header.entry_count = 0;
  limbo.capacity = 8;
  limbo.entries = malloc(limbo.capacity * sizeof(LimboEntry));
  read_limbo();
}

int find_entry(const char* entry_path) {
  for(size_t i=0; i<limbo.header.entry_count; i++) {
    if(strcmp(limbo.entries[i].path, entry_path) == 0) {
      return i;
    }
  }
  return -1;
}

bool equal_entries(const LimboEntry *a, const LimboEntry *b) {
    return a->flags       == b->flags &&
           a->mode        == b->mode &&
           a->mtime_sec   == b->mtime_sec &&
           a->mtime_nsec  == b->mtime_nsec &&
           a->fileSize    == b->fileSize &&
           strcmp(a->path, b->path) == 0;
}

bool add_limbo_entry(LimboEntry *entry) {
  int idx = find_entry(entry->path);
  if(idx >= 0) {
    if(!equal_entries(&limbo.entries[idx], entry)) {
      limbo.entries[idx] = *entry;
      return false;
    }
    return true;
  } else {
    if(limbo.header.entry_count == limbo.capacity) {
      limbo.capacity *= 2;
      limbo.entries = realloc(limbo.entries, limbo.capacity * sizeof(LimboEntry));
    }
    limbo.entries[limbo.header.entry_count++] = *entry;
    return false;
  }
}

size_t calc_limbo_buffer_size(uint32_t entry_count) {
  return sizeof(LimboHeader) + entry_count * sizeof(LimboEntry); 
}

void network_order_limbo() {
  for(int i=0; i<limbo.header.entry_count; i++) {
    LimboEntry *e = &limbo.entries[i];
    e->fileSize   = htonl(e->fileSize);
    e->mode       = htonl(e->mode);
    e->mtime_sec  = htonl(e->mtime_sec);
    e->mtime_nsec = htonl(e->mtime_nsec);
    e->flags      = htons(e->flags);
  }
  limbo.header.entry_count = htonl(limbo.header.entry_count);
}

void host_order_limbo() {
  for(int i=0; i<limbo.header.entry_count; i++) {
    LimboEntry *e = &limbo.entries[i];
    e->fileSize   = ntohl(e->fileSize);
    e->mode       = ntohl(e->mode);
    e->mtime_sec  = ntohl(e->mtime_sec);
    e->mtime_nsec = ntohl(e->mtime_nsec);
    e->flags      = ntohs(e->flags);
  }
}

void write_limbo() {
  char limbo_path[PATH_MAX];
  build_path(limbo_path, 3, YAGIT_SRC_DIR, YAGIT_DIR, LIMBO);
  FILE* limbo_file = fopen(limbo_path, "wb"); 

  size_t buffer_size = calc_limbo_buffer_size(limbo.header.entry_count);
  uint8_t *buffer = malloc(buffer_size + SHA256_DIGEST_SIZE);
  uint32_t entry_count = limbo.header.entry_count;
  network_order_limbo();
  memcpy(buffer, &limbo.header, sizeof(LimboHeader));
  memcpy(buffer + sizeof(LimboHeader), limbo.entries, entry_count * sizeof(LimboEntry));
  SHA256(buffer, buffer_size, limbo.checksum);
  memcpy(buffer + buffer_size, limbo.checksum, SHA256_DIGEST_SIZE);
  fwrite(buffer, buffer_size + SHA256_DIGEST_SIZE, 1, limbo_file);

  fclose(limbo_file);
}

void read_limbo() {
  char limbo_path[PATH_MAX];
  build_path(limbo_path, 3, YAGIT_SRC_DIR, YAGIT_DIR, LIMBO);
  struct stat st;
  if(stat(limbo_path, &st) == -1) return;

  size_t buffer_size = st.st_size;
  FILE* limbo_file = fopen(limbo_path, "rb");
  uint8_t *buffer = malloc(buffer_size);
  if (fread(buffer, 1, buffer_size, limbo_file) != buffer_size) {
    perror("Failed to read limbo file fully");
    free(buffer);
    fclose(limbo_file);
    return;
  }
  uint8_t calculated_checksum[SHA256_DIGEST_SIZE];
  SHA256(buffer, buffer_size - SHA256_DIGEST_SIZE, calculated_checksum);
  uint8_t actual_checksum[SHA256_DIGEST_SIZE];
  memcpy(actual_checksum, buffer + buffer_size - SHA256_DIGEST_SIZE, SHA256_DIGEST_SIZE);
  if(memcmp(actual_checksum, calculated_checksum, SHA256_DIGEST_SIZE) != 0) {
    printf("Tampering with the limbo file, that's treason. I, Curry the All Father, banish you until ur worthy enough.");
    return;
  }

  memcpy(&limbo.header, buffer, sizeof(LimboHeader));
  limbo.header.entry_count = ntohl(limbo.header.entry_count);
  limbo.capacity = limbo.header.entry_count * 2;
  limbo.entries = realloc(limbo.entries, limbo.capacity * sizeof(LimboEntry));
  memcpy(limbo.entries, buffer + sizeof(LimboHeader), limbo.header.entry_count * sizeof(LimboEntry));
  host_order_limbo();
  free(buffer);
  fclose(limbo_file);
}
