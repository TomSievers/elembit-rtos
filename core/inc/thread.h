#pragma once

#include "waker.h"
#include "stdint.h"

typedef void (*thread_entry_t)(void *);

typedef struct thread
{
    uint32_t state;
#define THREAD_STATE_ACTIVE 0x1
#define THREAD_STATE_WAIT_ON_WAKER 0x2
#define THREAD_STATE_WAKER_TIMEOUT 0x4
#define THREAD_STATE_JOINABLE 0x8
#define THREAD_STATE_RUNNING 0x10
    uint32_t priority;
    uint32_t stack_size;
    uint32_t id;
    void *stack;
    void *stack_start;
    void *arg;
    thread_entry_t entry;
    void *waker;

    uint32_t local_storage[4];
    volatile struct thread *next;
    volatile struct thread *prev;
#ifdef RR_SCHEDULE
    uint32_t time_slice_start;
    uint32_t consumed_time_slice;
    volatile struct thread *next_in_schedule;
    volatile struct thread *prev_in_schedule;
#endif

#ifdef MULTI_PROCESSING
    int32_t affinity;
    volatile void *spinlock;
#endif

#ifdef ASSYMMETRIC_MULTI_PROCESSING
    uint32_t performance_metric;
#endif
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
thread_t *thread_create(thread_entry_t entry, void *arg, uint32_t priority, void *stack, uint32_t stack_size);

#ifdef MULTI_PROCESSING
/**
 * @brief Create a thread on a specific core
 * @param core_id Core to run the thread on
 * @param entry Thread entry function
 * @param arg Argument to pass to the thread entry function
 * @param priority Thread priority
 * @param stack Pointer to the stack
 * @param stack_size Size of the stack
 * @return Pointer to the thread
 */
thread_t *thread_create_on_core(uint16_t core_id, thread_entry_t entry, void *arg, uint32_t priority, void *stack, uint32_t stack_size);
#endif // MULTI_PROCESSING

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
 * @return 0 if the thread was joined, -1 on error errno is set to ETIMEDOUT if the timeout was reached
 */
int thread_join(thread_t *thread, uint32_t timeout);
