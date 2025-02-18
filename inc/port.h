#pragma once

#include <stdint.h>

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
extern void thread_switch(void *thread);
