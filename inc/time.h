#pragma once

#include <stdint.h>

typedef void (*start_timer_t)(uint32_t millis);
typedef void (*stop_timer_t)();
typedef uint32_t (*get_time_t)();

typedef struct clock {
    get_time_t get_time;
    start_timer_t start_timer;
    stop_timer_t stop_timer;
} clock_t;

void time_init(clock_t* clock);

int32_t time_cmp(uint32_t start, uint32_t end);

uint32_t time_get();

void start_timer(uint32_t millis);

void stop_timer();