#include "chan.h"

#include "macros.h"

chan* chan_create(size_t capacity) {
  chan* c = LG_MALLOC_OR_DIE(sizeof(chan));
  chan_init(c, capacity);
  return c;
}

void chan_init(chan* c, size_t capacity) {
  c->data = LG_MALLOC_OR_DIE(capacity * sizeof(void*));
  c->capacity = capacity;
  c->reader = 0;
  c->writer = 0;
  c->readers_waiting = 0;
  c->writers_waiting = 0;
  c->full = false;
  pthread_mutex_init(&c->lock, NULL);
  pthread_cond_init(&c->rcond, NULL);
  pthread_cond_init(&c->wcond, NULL);
}

void chan_finalize(chan* c) {
  free(c->data);
  pthread_mutex_destroy(&c->lock);
  pthread_cond_destroy(&c->rcond);
  pthread_cond_destroy(&c->wcond);
}

void chan_destroy(chan* c) {
  chan_finalize(c);
  free(c);
}