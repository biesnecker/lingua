#pragma once

#include <stdlib.h>

#define LG_UNUSED(var) (void)((var))

#define LG_MALLOC_OR_DIE(sz)  \
  ({                          \
    void* ptr = malloc((sz)); \
    if (ptr == NULL) {        \
      abort();                \
    }                         \
    ptr;                      \
  })

#define LG_REALLOC_OR_DIE(ptr, newSize)       \
  ({                                          \
    void* newPtr = realloc((ptr), (newSize)); \
    if (newPtr == NULL) {                     \
      abort();                                \
    }                                         \
    newPtr;                                   \
  })