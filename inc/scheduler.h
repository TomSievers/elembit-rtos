#pragma once

#include <stdint.h>

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
 * @brief Register an idle thread.
 * @param thread The idle thread to register.
 */
void register_idle_thread(void *thread);

/**
 * @brief Reschedule the threads, should only be called from the supervisor.
 * This function will determine the next thread to run and switch to it.
 * If there are no threads to run, the idle thread will be with a timer that may interrupt the idle thread.
 * When this function is called ensure exclusive access to the scheduler.
 */
void reschedule();