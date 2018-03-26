#include "test.h"
#include "cache.h"

int test_failures;
int assert_count;

void test_cache_keys_bytes(struct cache *c, uint16_t expected_num, uint16_t expected_bytes) {
  uint16_t i;

  i=cache_numkeys(c);
  ASSERT(expected_num==i, "Number of elements in cache is mismatched");
  LOG(LL_INFO, ("Keys: Wanted %u, got %u", expected_num, i));

  i=cache_numbytes(c);
  ASSERT(expected_bytes==i, "Number of bytes in cache is mismatched");
  LOG(LL_INFO, ("Bytes: Wanted %u, got %u", expected_bytes, i));
}


void test_cache() {
  struct cache *c;
  bool ret;

  c=cache_create();
  ASSERT(c, "cache_create() failed");
  test_cache_keys_bytes(c, 0, 0);
  cache_loginfo(c);

  ret = cache_haskey(c, "Hello");
  ASSERT(!ret, "Key 'Hello' not expected in cache");
  cache_loginfo(c);

  ret = cache_addkey(c, "Hello");
  ASSERT(ret, "Could not add key to cache");
  test_cache_keys_bytes(c, 1, 6);
  cache_loginfo(c);

  ret = cache_addkey(c, "World");
  ASSERT(ret, "Could not add key to cache");
  test_cache_keys_bytes(c, 2, 12);
  cache_loginfo(c);

  ret = cache_haskey(c, "Hello");
  ASSERT(ret, "Key 'Hello' expected in cache");
  cache_loginfo(c);

  ret = cache_haskey(c, "World");
  ASSERT(ret, "Key 'World' expected in cache");
  cache_loginfo(c);

  ret = cache_haskey(c, "Hello World");
  ASSERT(!ret, "Key 'Hello World' not expected in cache");
  cache_loginfo(c);

  ret = cache_addkey(c, "Hello World");
  ASSERT(ret, "Could not add key to cache");
  test_cache_keys_bytes(c, 3, 24);
  cache_loginfo(c);

  ret = cache_haskey(c, "Hello World");
  ASSERT(ret, "Key 'Hello World' expected in cache");
  cache_loginfo(c);

  ret = cache_addkey(c, "testing123");
  ASSERT(ret, "Could not add key to cache");
  test_cache_keys_bytes(c, 4, 35);
  cache_loginfo(c);

  ret = cache_haskey(c, "Hello World");
  ASSERT(ret, "Key 'Hello World' expected in cache");
  cache_loginfo(c);

  ret = cache_destroy(&c);
  ASSERT(ret, "cache_destroy() failed");
  ASSERT(!c, "cache object was not NULL");
  cache_loginfo(c);
}

int main() {
  test_cache();
    if (test_failures) {
    LOG(LL_ERROR, ("%d test failures", test_failures));
    return -1;
  }
  LOG(LL_INFO, ("All tests passed, %d assertions OK", assert_count));
  return 0;
}
