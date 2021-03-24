#include "chan.h"

#include <assert.h>
#include <pthread.h>

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
  c->closed = false;
  pthread_mutex_init(&c->lock, NULL);
  pthread_cond_init(&c->rcond, NULL);
  pthread_cond_init(&c->wcond, NULL);
}

void chan_close(chan* c) {
  pthread_mutex_lock(&c->lock);

  if (c->closed) {
    // Can't close an already closed channel.
    pthread_mutex_unlock(&c->lock);
    assert(false);
  } else {
    c->closed = true;
    pthread_cond_broadcast(&c->rcond);
    pthread_cond_broadcast(&c->wcond);
  }

  pthread_mutex_unlock(&c->lock);
}

bool chan_is_closed(chan* c) { return c->closed; }

size_t chan_capacity(chan* c) { return c->capacity; }

size_t chan_size(chan* c) {
  if (c->reader == c->writer) {
    // It's either at capacity or empty.
    return c->full ? c->capacity : 0;
  } else if (c->writer > c->reader) {
    // Gap between the reader and writer.
    return c->writer - c->reader;
  } else {
    // Distance from the reader to the end, plus distance from the beginning
    // to the writer.
    return c->capacity - c->reader + c->writer;
  }
}

bool chan_empty(chan* c) { return (c->writer == c->reader) && !c->full; }

void chan_write(chan* c, void* data) {
  pthread_mutex_lock(&c->lock);

  if (c->closed) {
    pthread_mutex_unlock(&c->lock);
    // Can't write to a closed channel.
    assert(false);
  }

  while (c->full) {
    c->writers_waiting += 1;
    pthread_cond_wait(&c->wcond, &c->lock);
    c->writers_waiting -= 1;
  }

  c->data[c->writer++] = data;

  // Loop to the front if we've gone past the end.
  if (c->writer == c->capacity) {
    c->writer = 0;
  }

  c->full = c->writer == c->reader;
  if (c->readers_waiting > 0) {
    pthread_cond_signal(&c->rcond);
  }

  pthread_mutex_unlock(&c->lock);
}

void* chan_read(chan* c) {
  pthread_mutex_lock(&c->lock);

  while (chan_empty(c)) {
    if (c->closed) {
      // If it closed while we were waiting and there's nothing else to read,
      // we'll just get NULL, and future reads will abort.
      pthread_mutex_unlock(&c->lock);
      return NULL;
    }
    c->readers_waiting += 1;
    pthread_cond_wait(&c->rcond, &c->lock);
    c->readers_waiting -= 1;
  }

  void* data = c->data[c->reader++];

  if (c->reader == c->capacity) {
    c->reader = 0;
  }

  c->full = false;
  if (c->writers_waiting > 0) {
    pthread_cond_signal(&c->wcond);
  }

  pthread_mutex_unlock(&c->lock);
  return data;
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
