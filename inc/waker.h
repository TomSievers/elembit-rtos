#pragma once

#include <stdint.h>

typedef int (*poll_fn)(void *waker);

typedef enum waker_type
{
    WAKER_EVENT,
    WAKER_TIMED
} waker_type_t;

typedef struct event_waker
{
    waker_type_t waker_type;
    poll_fn poll;
    void* context;
    struct waker *next;
} event_waker_t;

typedef struct timed_waker
{
    waker_type_t waker_type;
    poll_fn poll;
    void* context;
    struct waker *next;
    uint32_t timeout;
    uint32_t start_time;
} timed_waker_t;

/**
 * @brief Initialize a timed waker
 * @param waker Pointer to the timed waker
 * @param poll Poll function to call when the waker is polled
 * @param context Context to pass to the poll function
 * @param timeout Timeout in milliseconds
 * @note The poll function should return 0 if the event is ready, otherwise 1
 */
void init_timed_waker(timed_waker_t *waker, poll_fn poll, void* context, uint32_t timeout);

/**
 * @brief Initialize an event waker
 * @param waker Pointer to the event waker
 * @param poll Poll function to call when the waker is polled
 * @param context Context to pass to the poll function
 * @note The poll function should return 0 if the event is ready, otherwise 1
 */
void init_event_waker(event_waker_t *waker, poll_fn poll, void* context);