#include "mgos.h"
#include "cache.h"

struct cache *cache_create(void) {
  struct cache *c = calloc(1, sizeof(struct cache));

  if (!c) return NULL;

  c->num=0;
  c->len=0;
  c->string=NULL;
  return c;
}

bool cache_haskey(struct cache *cache, const char *key) {
  if (!cache || !cache->string) return false;
  return (NULL != strstr(cache->string, key));
}

uint16_t cache_numkeys(struct cache *cache) {
  if (!cache) return 0;
  return cache->num;
}

uint16_t cache_numbytes(struct cache *cache) {
  if (!cache) return 0;
  return cache->len;
}

bool cache_addkey(struct cache *cache, const char *key) {
  size_t keylen;

  keylen=strlen(key);

  if (0 == cache->len) {
//    LOG(LL_DEBUG, ("Adding key '%s' to empty cache", key));
    cache->string=malloc(keylen+1);
    cache->len=keylen+1;
    memcpy(cache->string, key, keylen);
    cache->string[cache->len-1]='\0';
  } else {
//    LOG(LL_DEBUG, ("Adding key '%s' to cache '%s'", key, cache->string));
    cache->string[cache->len-1]=CACHE_SEPARATOR;
    cache->string=realloc(cache->string, cache->len+keylen+1);
    memcpy(cache->string+cache->len, key, keylen);
    cache->len+=keylen+1;
    cache->string[cache->len-1]='\0';
  }
  cache->num++;
  return true;
}

void cache_loginfo(struct cache *c) {
  if (!c) return;
  LOG(LL_INFO, ("Cache is now '%s', %d elem(s), %u byte(s)", c->string, c->num, c->len));
}

bool cache_destroy(struct cache **cache) {
  if (!*cache) return false;

  if ((*cache)->string) free ((*cache)->string);

  free(*cache);
  *cache=NULL;
  return true;
}

