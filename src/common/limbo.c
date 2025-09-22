#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "limbo.h"
#include "platform.h"

Limbo limbo;

void limbo_init() {
  strcpy(limbo.header.magic, "LMBO");
  limbo.header.entry_count = 0;
  limbo.capacity = 8;
  limbo.entries = malloc(limbo.capacity * sizeof(LimboEntry));
}

void add_limbo_entry(LimboEntry entry) {
  if(limbo.header.entry_count == limbo.capacity) {
    limbo.capacity *= 2;
    limbo.entries = realloc(limbo.entries, limbo.capacity * sizeof(LimboEntry));
  }
  limbo.entries[limbo.header.entry_count++] = entry;
}

size_t calc_limbo_buffer_size() {
  return sizeof(LimboHeader) + limbo.header.entry_count * sizeof(LimboEntry); 
}

void network_order_limbo() {
  for(int i=0; i<limbo.header.entry_count; i++) {
    limbo.entries->flags = htonl(limbo.entries->fileSize); 
    limbo.entries->mode = htonl(limbo.entries->mode); 
    limbo.entries->mtime_sec = htonl(limbo.entries->mtime_sec); 
    limbo.entries->mtime_nsec = htonl(limbo.entries->mtime_nsec); 
  }
  limbo.header.entry_count = htonl(limbo.header.entry_count);
}

void host_order_limbo() {
  limbo.header.entry_count = ntohl(limbo.header.entry_count);
  for(int i=0; i<limbo.header.entry_count; i++) {
    limbo.entries->flags = ntohl(limbo.entries->fileSize); 
    limbo.entries->mode = ntohl(limbo.entries->mode); 
    limbo.entries->mtime_sec = ntohl(limbo.entries->mtime_sec); 
    limbo.entries->mtime_nsec = ntohl(limbo.entries->mtime_nsec); 
  }
}
