#pragma once

#include "semaphore.h"
#include <stdint.h>

typedef semaphore_t mutex_t;

/**
 * @brief Initialize a mutex, this is NOT thread safe.
 * @param mutex Mutex to initialize
 */
void mutex_init(mutex_t *mutex);

/**
 * @brief Lock a mutex, this will block the current thread until the mutex is available.
 * @param mutex Mutex to lock
 * @param timeout Timeout in milliseconds
 * @return 0 if the mutex was locked, -1 if the timeout was reached
 */
int mutex_lock(mutex_t *mutex, uint32_t timeout);

/**
 * @brief Unlock a mutex, this will unblock a thread waiting on the mutex.
 * @param mutex Mutex to unlock
 */
void mutex_unlock(mutex_t *mutex);