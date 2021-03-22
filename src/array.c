#include "array.h"

#include <assert.h>
#include <string.h>

array* array_create(size_t cap, size_t elemSize) {
  array* arr = LG_MALLOC_OR_DIE(sizeof(array));
  array_init(arr, cap, elemSize);
  return arr;
}

void array_init(array* arr, size_t cap, size_t elemSize) {
  size_t capBytes = cap * elemSize;
  buffer_init(&arr->b, capBytes);
  arr->elemSize = elemSize;
}

size_t array_size(array* arr) { return arr->b.cursor / arr->elemSize; }

void* array_at(array* arr, size_t idx) {
  assert(idx < array_size(arr));
  size_t byteIdx = idx * arr->elemSize;
  return (char*)arr->b.data + byteIdx;
}

void* array_push(array* arr, const void* elem) {
  return buffer_append_bytes(&arr->b, elem, arr->elemSize);
}

void* array_set(array* arr, size_t idx, const void* elem) {
  size_t arraySize = array_size(arr);
  assert(idx <= arraySize);

  // If it's the next index past the end push it.
  if (idx == arraySize) {
    return array_push(arr, elem);
  }

  void* dest = array_at(arr, idx);
  memcpy(dest, elem, arr->elemSize);
  return dest;
}

void array_finalize(array* arr) { buffer_finalize(&arr->b); }

void array_destroy(array* arr) {
  array_finalize(arr);
  free(arr);
}