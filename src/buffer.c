#include "buffer.h"

#include <string.h>

#include "macros.h"

buffer* buffer_create(size_t cap) {
  buffer* b = LG_MALLOC_OR_DIE(sizeof(buffer));
  buffer_init(b, cap);
  return b;
}

void buffer_init(buffer* b, size_t cap) {
  b->data = cap > 0 ? LG_MALLOC_OR_DIE(cap) : NULL;
  b->capacity = cap;
  b->cursor = 0;
}

static void _buffer_grow(buffer* b, size_t newCap) {
  if (b->capacity >= newCap) {
    return;
  }
  b->data = LG_REALLOC_OR_DIE(b->data, newCap);
  b->capacity = newCap;
}

static void* buffer_at_cursor(buffer* b) { return (char*)b->data + b->cursor; }

void* buffer_append_bytes(buffer* b, const void* src, size_t sz) {
  _buffer_grow(b, b->cursor + sz);
  void* writeStart = buffer_at_cursor(b);
  memcpy(writeStart, src, sz);
  b->cursor += sz;
  return writeStart;
}

void buffer_finalize(buffer* b) {
  free(b->data);
  b->data = NULL;
  b->capacity = 0;
  b->cursor = 0;
}

void buffer_destroy(buffer* b) {
  buffer_finalize(b);
  free(b);
}