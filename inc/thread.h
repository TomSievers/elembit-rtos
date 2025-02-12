#pragma once

#include "waker.h"
#include "stdint.h"

typedef void (*thread_entry_t)(void *);

typedef struct thread
{
    uint32_t state;
#define THREAD_STATE_RUNNING 0x1
#define THREAD_STATE_WAIT_ON_WAKER 0x2
#define THREAD_STATE_WAKER_TIMEOUT 0x4
#define THREAD_STATE_JOINABLE 0x8
    uint32_t priority;
    uint32_t stack_size;
    void *stack;
    void *stack_start;
    void *arg;
    thread_entry_t entry;
    void *waker;
    struct thread *next;
} thread_t;

/**
 * @brief Initialize a thread
 * @param entry Thread entry function
 * @param arg Argument to pass to the thread entry function
 * @param priority Thread priority
 * @param stack Pointer to the stack
 * @param stack_size Size of the stack
 * @return Pointer to the thread
 */
thread_t *thread_init(thread_entry_t entry, void *arg, uint32_t priority, void *stack, uint32_t stack_size);

/**
 * @brief Get a pointer to the current thread if we are running in a thread context
 * @return Pointer to the current thread or NULL if we are not running in a thread context
 */
thread_t *thread_current();

/**
 * @brief Sleep the current thread for AT LEAST the specified timeout
 * @param timeout Timeout in milliseconds
 */
void thread_sleep(uint32_t timeout);

/**
 * @brief Try to join the thread or timeout
 * @param thread Thread to join
 * @param timeout Timeout in milliseconds
 * @return 0 if the thread was joined, 1 if the timeout was reached, -1 if the thread is not running
 */
int thread_join(thread_t *thread, uint32_t timeout);