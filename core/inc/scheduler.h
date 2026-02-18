#pragma once

#include <stdint.h>
#include <thread.h>
#include <port.h>

#ifdef ASSYMMETRIC_MULTI_PROCESSING

typedef struct core_info
{
    uint32_t core_id;            // The core id of the core
    uint32_t performance_metric; // The performance metric of the core (higher is better)
} core_info_t;

#endif // ASSYMMETRIC_MULTI_PROCESSING

/**
 * @brief Initialize the scheduler.
 * This function should be called before any other scheduler functions. (Or thread functions for that matter)
 * This function will initialize the idle thread and should be called from where the idle thread is located (the main entry point most likely).
 */
void scheduler_init();

/**
 * @brief Yield the current thread and schedule another thread to run (This may be the same thread if no other thread needs to be run).
 */
void yield();

/**
 * @brief Register a waker for the current thread.
 * @param waker The waker to register.
 */
void register_waker(void *waker);

/**
 * @brief Unregister the waker for the current thread.
 */
void unregister_waker();

/**
 * @brief Register a thread to be scheduled.
 * @param thread The thread to register.
 */
void register_thread(void *thread);

/**
 * @brief Unregister a thread from the scheduler.
 * @param thread The thread to unregister.
 */
void unregister_thread(volatile void *thread);

/**
 * @brief Reschedule the threads, should only be called from the supervisor.
 * This function will determine the next thread to run and switch to it.
 * If there are no threads to run, the idle thread will be with a timer that may interrupt the idle thread.
 * When this function is called ensure exclusive access to the scheduler.
 */
void reschedule();

/**
 * @brief Entry point for a thread.
 * This function will call the thread entry function and mark the thread as joinable when it has finished executing
 * This function should never return.
 */
void thread_entry();

/**
 * @brief Lock the current thread from multi core access.
 * @param thread The thread to lock.
 * @note This function should be called when the thread is being modified.
 */
inline void lock_thread(volatile thread_t *thread);

/**
 * @brief Unlock the current thread for multi core access.
 * @param thread The thread to unlock.
 * @note This function should be called when the thread is no longer being modified.
 */
inline void unlock_thread(volatile thread_t *thread);

#ifdef ASSYMMETRIC_MULTI_PROCESSING
/**
 * @brief Get information about the current core.
 * @return A pointer to the core info structure.
 */
const core_info_t *get_core_info(uint32_t core_id);
#endif

#ifdef MULTI_PROCESSING

inline void lock_thread(volatile thread_t *thread)
{
    mp_spinlock_lock(thread->spinlock);
}
inline void unlock_thread(volatile thread_t *thread)
{
    mp_spinlock_unlock(thread->spinlock);
}
#else
inline void lock_thread(volatile thread_t *thread)
{
    (void)thread;
}
inline void unlock_thread(volatile thread_t *thread)
{
    (void)thread;
}
#endif
