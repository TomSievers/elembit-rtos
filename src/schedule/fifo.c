#include "scheduler.h"
#include "thread.h"
#include "waker.h"
#include "time.h"
#include "port.h"
#include <stddef.h>

#if !defined(ROUND_ROBIN)

extern thread_t* is_thread_runnable(volatile thread_t* cur, uint32_t* sleep_time);

volatile thread_t* determine_next_thread(volatile thread_t* thread_list, uint32_t* sleep_time)
{
    volatile thread_t *last_thread = get_thread_pointer();

    if (last_thread != NULL)
    {
        lock_thread(last_thread);

        last_thread->state &= ~THREAD_STATE_RUNNING;

        unlock_thread(last_thread);
    }

    volatile thread_t* cur = thread_list;

    *sleep_time = 0xFFFFFFFF;

    while (cur != NULL)
    {
        // A joineable thread will never be run again remove it from the thread list.
        if (cur->state & THREAD_STATE_JOINABLE)
        {
            unregister_thread(cur);
        }
        else
        {
            // Check if we are allowed to run the thread
#ifdef MP
            if (cur->affinity == -1 || cur->affinity == core_id())
#endif
            {
                lock_thread(cur);

                volatile thread_t *ret = is_thread_runnable(cur, sleep_time);

                if (ret != NULL)
                {
                    // Indicate to possibly another core that we are running this thread
                    ret->state |= THREAD_STATE_RUNNING;
                }

                unlock_thread(cur);

                if (ret != NULL)
                {
                    return ret;
                }
            }
        }

        cur = cur->next;
    }

    // There are no threads to run, sleep for the sleep time or forever.
    // An event may wake up the system earlier and cause a thread to run.
    return NULL;
}

#endif