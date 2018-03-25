#ifndef __TEST_H
#define __TEST_H

#include <stdlib.h>
#include <string.h>
#include "mgos_mock.h"

extern int test_failures;
extern int assert_count;

#define ASSERT(expr, errstr)               \
  do {                                     \
    if (!(expr)) {  \
      LOG(LL_ERROR, ("ASSERT FAIL: "errstr));  \
      test_failures++;                       \
    } \
    assert_count++; \
  } while (0)



#endif // __TEST_H
