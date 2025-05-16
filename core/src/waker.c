#include "waker.h"
#include "time.h"

void init_timed_waker(timed_waker_t *waker, poll_fn poll, void *context, uint32_t timeout)
{
    waker->waker_type = WAKER_TIMED;
    waker->poll = poll;
    waker->context = context;
    waker->timeout = timeout;
    waker->start_time = time_get();
}

void init_event_waker(event_waker_t *waker, poll_fn poll, void *context)
{
    waker->waker_type = WAKER_EVENT;
    waker->poll = poll;
    waker->context = context;
}
