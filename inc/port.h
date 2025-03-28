#pragma once

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Get the current stack pointer
 * @return Pointer to the current stack
 */
extern uint32_t *get_stack_pointer();

#define SUPERVISOR_FUNC_RESCHEDULE 0
/**
 * @brief Jump to the supervisor function
 * @param function Function to jump to
 * @note This function should store the processor state before jumping to the supervisor function
 */
extern void jump_to_supervisor(uint32_t function);

/**
 * @brief Get the current thread pointer
 * @return Pointer to the current thread
 */
extern void *get_thread_pointer();

/**
 * @brief Set the current thread pointer
 * @param thread Pointer to the thread
 */
extern void set_thread_pointer(void *thread);

/**
 * @brief Enter a critical section
 * @return State of the processor before entering the critical section
 */
extern uint32_t enter_critical_section();

/**
 * @brief Exit a critical section
 * @param state State of the processor before entering the critical section
 */
extern void exit_critical_section(uint32_t state);

/**
 * @brief Switch to a new thread
 * @param thread Pointer to the thread to switch to
 */
extern void thread_switch(volatile void *thread);

#ifdef ROUND_ROBIN

/**
 * @brief Schedule the end of the next time slice in x millis. At the end of the time slice, the scheduler will be called.
 * @param millis Milliseconds till the current time slice ends.
 * @note On multi-processor systems, this needs to be independent for each core.
 */
extern void schedule_time_slice_end(uint32_t millis);

#endif

#ifdef MP

/**
 * @brief Acquire a spinlock for a multi-processor environment
 * @param exclusive True if the lock MUST be exclusive, false if a shared spinlock is allowed
 * @return Pointer to the spinlock
 * @note At least 2 exclusive spinlocks must be available.
 */
extern volatile void* mp_acquire_spinlock(bool exclusive);

/**
 * @brief Try to lock a spinlock, blocks if the lock is already taken
 * @param spinlock Spinlock to lock
 */
extern void mp_spinlock_lock(volatile void* spinlock);

/**
 * @brief Unlock a acquired spinlock
 * @param spinlock Spinlock to lock
 */
extern void mp_spinlock_unlock(volatile void *spinlock);

/**
 * @brief Get the core id of where the function is running
 * @return uint16_t Core id
 */
extern uint16_t core_id();

#endif
