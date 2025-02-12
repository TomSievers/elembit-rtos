#pragma once

#include <stdint.h>

extern uint32_t *get_stack_pointer();

#define SUPERVISOR_FUNC_RESCHEDULE 0
extern void jump_to_supervisor(uint32_t function);

extern void *get_thread_pointer();

extern void set_thread_pointer(void *thread);

extern uint32_t enter_critical_section();

extern void exit_critical_section(uint32_t state);

extern void thread_switch(void *thread);
