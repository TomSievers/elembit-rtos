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

#define TIMEOUT_INF 0xFFFFFFFF

/**
 * @brief Initialize the time module using the provided clock
 * @param clock Pointer to the clock struct
 */
void time_init(clock_t* clock);

/**
 * @brief Get the number of milliseconds passed since the specified time
 * @param start Start time
 * @return Number of milliseconds passed since the start time
 * @note This function will account for overflow
 */
uint32_t time_millis_passed_since(uint32_t start);

/**
 * @brief Get the current time in milliseconds
 * @return Current time in milliseconds
 */
uint32_t time_get();

/**
 * @brief Start a timer that will cause an interrupt after the specified number of milliseconds
 * @param millis Number of milliseconds to wait before the timer expires
 * @note This function will call the start_timer function of the clock struct
 */
void time_start_timer(uint32_t millis);

/**
 * @brief Stop the timer
 * @note This function will call the stop_timer function of the clock struct
 */
void time_stop_timer();