#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include "sha256.h"

typedef struct {
  char magic[4];
  uint32_t entry_count;
} LimboHeader;

typedef struct {
  uint32_t mtime_sec;
  uint32_t mtime_nsec;  
  uint32_t mode;
  uint32_t fileSize;
  uint16_t flags;
  uint8_t hash[SHA256_DIGEST_SIZE];
  char path[PATH_MAX];
} LimboEntry;

typedef struct {
  LimboHeader header;
  LimboEntry* entries;
  uint8_t checksum[SHA256_DIGEST_SIZE];
  size_t capacity;
} Limbo;

extern Limbo limbo;
void limbo_init();
void add_limbo_entry(LimboEntry entry);
size_t calc_limbo_buffer_size();
void network_order_limbo();
void host_order_limbo();
