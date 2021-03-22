#pragma once

#include "buffer.h"
#include "macros.h"

typedef struct array {
  buffer b;
  size_t elemSize;
} array;

array* array_create(size_t cap, size_t elemSize);

void array_init(array* arr, size_t cap, size_t elemSize);

size_t array_size(array* arr);

void* array_at(array* arr, size_t idx);

void* array_push(array* arr, const void* elem);

void* array_set(array* arr, size_t idx, const void* elem);

void array_finalize(array* arr);

void array_destroy(array* arr);