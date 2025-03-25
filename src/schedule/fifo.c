#include "scheduler.h"
#include "thread.h"
#include "waker.h"
#include "time.h"
#include "port.h"
#include <stddef.h>

#if !defined(ROUND_ROBIN)

thread_t* determine_next_thread(thread_t* thread_list, uint32_t* sleep_time)
{
    thread_t* cur = thread_list;

    *sleep_time = 0xFFFFFFFF;

    while (cur != NULL)
    {
        // Check if the thread is NOT joinable
        if (!cur->state & (THREAD_STATE_JOINABLE))
        {
            // Check if the thread is waiting on a waker
            if (cur->state & THREAD_STATE_WAIT_ON_WAKER)
            {
                // Check if the waker is ready
                if (cur->waker != NULL)
                {
                    event_waker_t *waker = (event_waker_t *)cur->waker;

                    switch (waker->waker_type)
                    {
                    case WAKER_EVENT:
                    {
                        int result = waker->poll(cur->waker);

                        if (result == 0)
                        {
                            cur->state &= ~THREAD_STATE_WAIT_ON_WAKER;
                            cur->waker = NULL;
                            return cur;
                        }
                        break;  
                    }
                    case WAKER_TIMED:
                    {
                        timed_waker_t *twaker = (timed_waker_t *)cur->waker;

                        int result = 1;

                        if (twaker->poll != NULL)
                        {
                            result = twaker->poll(cur->waker);
                        }

                        // Check if the waker is ready
                        if (result == 0)
                        {
                            cur->state &= ~THREAD_STATE_WAIT_ON_WAKER;
                            cur->waker = NULL;
                            return cur;
                        }
                        // Check if the waker has timed out
                        else if (time_millis_passed_since(twaker->start_time) > twaker->timeout)
                        {
                            cur->state &= ~THREAD_STATE_WAIT_ON_WAKER;
                            cur->state |= THREAD_STATE_WAKER_TIMEOUT;
                            cur->waker = NULL;
                            return cur;
                        }
                        // Update the sleep time with a new minimum
                        else if (*sleep_time == 0 || time_millis_passed_since(twaker->start_time) < *sleep_time)
                        {
                            *sleep_time = time_millis_passed_since(twaker->start_time);
                        }
                        break;
                    }
                    default:
                        break;
                    }
                }
            }
            else
            {
                return cur;
            }
        }

        cur = cur->next;
    }

    // There are no threads to run, sleep for the sleep time or forever.
    // An event may wake up the system earlier and cause a thread to run.
    return NULL;
}

#endif