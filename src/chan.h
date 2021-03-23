#pragma once

#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct chan {
  void** data;
  size_t capacity;
  size_t reader;
  size_t writer;
  size_t readers_waiting;
  size_t writers_waiting;
  pthread_mutex_t lock;
  pthread_cond_t rcond;
  pthread_cond_t wcond;
  bool full;
} chan;

chan* chan_create(size_t capacity);

void chan_init(chan* c, size_t capacity);

void chan_finalize(chan* c);

void chan_destroy(chan* c);