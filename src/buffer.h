#pragma once

#include <stdlib.h>

typedef struct buffer {
  void* data;
  size_t capacity;
  size_t cursor;
} buffer;

buffer* buffer_create(size_t cap);

void buffer_init(buffer* b, size_t cap);

void* buffer_append_bytes(buffer* b, const void* src, size_t sz);

void buffer_finalize(buffer* b);

void buffer_destroy(buffer* b);