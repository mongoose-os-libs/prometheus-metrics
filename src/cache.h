#pragma once

#include <stdint.h>
#include <stdbool.h>

#define CACHE_SEPARATOR    0x1E         // ASCII character for "Record Separator"

struct cache {
  char *   string;
  uint16_t len;
  uint16_t num;
};

struct cache *cache_create(void);
bool cache_haskey(struct cache *cache, const char *key);
bool cache_addkey(struct cache *cache, const char *key);
uint16_t cache_numkeys(struct cache *cache);
uint16_t cache_numbytes(struct cache *cache);
void cache_loginfo(struct cache *cache);
bool cache_destroy(struct cache **cache);
