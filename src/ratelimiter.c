#include "ratelimiter.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#include "macros.h"

#define NS_PER_SEC 1000000000ULL

static double _ratelimiter_now_seconds(void) {
  struct timespec now;
  if (clock_gettime(CLOCK_MONOTONIC_RAW, &now) == -1) {
    abort();
  };
  size_t nanoseconds = (size_t)((now.tv_sec * NS_PER_SEC) + now.tv_nsec);
  return nanoseconds / (double)(NS_PER_SEC);
}

ratelimiter* ratelimiter_create(double rps) {
  assert(rps >= 1.0);
  ratelimiter* r = LG_MALLOC_OR_DIE(sizeof(ratelimiter));
  ratelimiter_init(r, rps);
  return r;
}

void ratelimiter_init(ratelimiter* r, double rps) {
  assert(rps >= 1.0);
  r->rps = rps;
  r->allowance = rps;
  r->lastcheck = _ratelimiter_now_seconds();
  pthread_mutex_init(&r->lock, NULL);
  r->pending = 0;
}

double _ratelimiter_acquire_impl(ratelimiter* r, bool isNewRequest,
                                 bool* success) {
  pthread_mutex_lock(&r->lock);
  double current = _ratelimiter_now_seconds();
  double time_passed = current - r->lastcheck;
  r->lastcheck = current;
  r->allowance += time_passed * r->rps;
  if (r->allowance > r->rps) {
    r->allowance = r->rps;
  }
  if (isNewRequest) {
    r->pending += 1;
  }
  if (r->allowance < 1.0) {
    double next = ((1.0 - r->allowance) / r->rps) + ((r->pending - 1) / r->rps);
    pthread_mutex_unlock(&r->lock);
    *success = false;
    return next;
  } else {
    assert(r->pending > 0);
    r->pending -= 1;
    r->allowance -= 1.0;
    pthread_mutex_unlock(&r->lock);
    *success = true;
    return 0.0;
  }
}

void ratelimiter_acquire(ratelimiter* r) {
  bool isNewRequest = true;
  bool res;
  while (true) {
    double sleepSeconds = _ratelimiter_acquire_impl(r, isNewRequest, &res);
    if (res) {
      break;
    }
    isNewRequest = false;
    unsigned sleepMicros = (unsigned)(sleepSeconds * 1000000U);
    usleep(sleepMicros);
  }
}

void ratelimiter_finalize(ratelimiter* r) { pthread_mutex_destroy(&r->lock); }

void ratelimiter_destroy(ratelimiter* r) {
  ratelimiter_finalize(r);
  free(r);
}

#undef NS_PER_SEC