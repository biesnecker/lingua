#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "array.h"
#include "buffer.h"
#include "chan.h"
#include "macros.h"
#include "ratelimiter.h"

#define THREAD_COUNT 10

pthread_mutex_t print_lock;
int cnt = 0;

void* thread_job(void* arg) {
  ratelimiter* r = (ratelimiter*)arg;
  unsigned long tid = (unsigned long)(pthread_self());
  ratelimiter_acquire(r);
  pthread_mutex_lock(&print_lock);
  cnt += 1;
  printf("%lu: acquired ratelimit: %d\n", tid, cnt);
  pthread_mutex_unlock(&print_lock);
  return NULL;
}

void* thread_chan_job(void* arg) {
  chan* c = (chan*)arg;
  while (true) {
    int* payload = chan_read(c);
    if (payload == NULL) {
      break;
    }
    pthread_mutex_lock(&print_lock);
    printf("Got %d from chan\n", *payload);
    pthread_mutex_unlock(&print_lock);
    usleep(*payload);
  }
  return NULL;
}

int main(int argc, char** argv) {
  LG_UNUSED(argc);
  LG_UNUSED(argv);

  ratelimiter r;
  ratelimiter_init(&r, 5);

  pthread_mutex_init(&print_lock, NULL);

  pthread_t tid[THREAD_COUNT] = {0};
  for (int i = 0; i < THREAD_COUNT; ++i) {
    int error = pthread_create(&(tid[i]), NULL, &thread_job, &r);
    if (error) {
      abort();
    }
  }

  for (int i = 0; i < THREAD_COUNT; ++i) {
    pthread_join(tid[i], NULL);
  }

  pthread_mutex_destroy(&print_lock);

  ratelimiter_finalize(&r);

  buffer b;
  buffer_init(&b, 30);

  char message1[] = "This is a test ";
  char message2[] = "ya jerk";
  char message3[] = "!!!";

  buffer_append_bytes(&b, message1, strlen(message1));
  buffer_append_bytes(&b, message2, strlen(message2));
  buffer_append_bytes(&b, message3, strlen(message3) + 1);

  printf("Message: %s\n", (char*)b.data);
  printf("%zu %zu\n", b.capacity, b.cursor);

  buffer_finalize(&b);

  array intArray;
  array_init(&intArray, 10, sizeof(int));

  for (int i = 0; i < 10; ++i) {
    array_push(&intArray, &i);
  }

  int newValue = 42;
  array_set(&intArray, 8, &newValue);

  for (size_t i = 0; i < array_size(&intArray); ++i) {
    int* val = (int*)array_at(&intArray, i);
    printf("Value at idx %zu is %d\n", i, *val);
  }

  array_finalize(&intArray);

  chan c;
  chan_init(&c, 10);

  int x = 42;
  for (size_t i = 0; i < chan_capacity(&c); ++i) {
    printf("Writing to chan\n");
    chan_write(&c, &x);
  }
  for (size_t i = 0; i < chan_capacity(&c); ++i) {
    int* res = chan_read(&c);
    printf("From chan: %d\n", *res);
  }
  printf("Chan size: %zu\n", chan_size(&c));

  pthread_t num_reader;
  int error = pthread_create(&num_reader, NULL, &thread_chan_job, &c);
  if (error) {
    abort();
  }
  int nums[100] = {0};
  for (int i = 0; i < 100; ++i) {
    nums[i] = 1000 + i;
    chan_write(&c, &(nums[i]));
    pthread_mutex_lock(&print_lock);
    printf("Wrote %d: %d\n", i, nums[i]);
    pthread_mutex_unlock(&print_lock);
    if (i % 23 == 0) {
      sleep(1);
    }
  }

  chan_close(&c);

  pthread_join(num_reader, NULL);

  chan_finalize(&c);

  return EXIT_SUCCESS;
}
