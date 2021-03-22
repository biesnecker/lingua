#pragma once

#include <stdlib.h>
#include <pthread.h> 

typedef struct ratelimiter {
    double rps;
    double allowance;
    double lastcheck;
    pthread_mutex_t lock;
    unsigned pending;
} ratelimiter;

ratelimiter* ratelimiter_create(double rps);
void ratelimiter_init(ratelimiter* r, double rps);

// Sleeps the current thread until it's able to acquire a ticket. May be unfair.
void ratelimiter_acquire(ratelimiter* r);

void ratelimiter_finalize(ratelimiter* r);
void ratelimiter_destroy(ratelimiter* r);