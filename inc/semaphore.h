#pragma once

#include <stdint.h>

typedef uint32_t semaphore_t;

/**
 * @brief Initialize a semaphore, this is NOT thread safe.
 * @param sem Semaphore to initialize
 * @param count Initial count of the semaphore
 * @return 0 on success, -1 on error and errno is set
 */
int semaphore_init(semaphore_t *sem, uint32_t count);

/**
 * @brief Wait on a semaphore, this will block the current thread until the semaphore is signaled or a timeout occurs.
 * @param sem Semaphore to wait on
 * @param timeout Timeout in milliseconds
 * @return 0 if the semaphore was signaled, -1 if a timeout occurred (errno is set to ETIMEDOUT)
 */
int semaphore_wait(semaphore_t *sem, uint32_t timeout);

/**
 * @brief Signal a semaphore, this will unblock a thread waiting on the semaphore. 
 * (Although this function is safe to be called form an ISR, it is not ideal because it will cause a context switch to occur)
 * @param sem Semaphore to signal
 */
void semaphore_signal(semaphore_t *sem);

/**
 * @brief Signal a semaphore from an ISR, this will unblock a thread waiting on the semaphore. 
 * This function is safe to be called from an ISR and does not force a reschedule to occur immediatly.
 * @param sem Semaphore to signal
 */
void semaphore_signal_from_isr(semaphore_t *sem);
