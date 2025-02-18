#include "time.h"

static clock_t* global_clock;

void time_init(clock_t* clock)
{
    global_clock = clock;
}

uint32_t time_millis_passed_since(uint32_t start)
{
    uint32_t end = time_get();

    // Check for overflow
    if (start > end)
    {
        return UINT32_MAX - start + end;
    }

    return end - start;
}

uint32_t time_get()
{
    return global_clock->get_time();
}

void time_start_timer(uint32_t millis)
{
    global_clock->start_timer(millis);
}

void time_stop_timer()
{
    global_clock->stop_timer();
}

