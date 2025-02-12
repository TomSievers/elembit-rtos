#include "scheduler.h"
#include "thread.h"
#include "waker.h"
#include "time.h"
#include "port.h"
#include <stddef.h>

static thread_t* thread_list = NULL;
static thread_t* idle_thread = NULL;

thread_t* determine_next_thread(uint32_t* sleep_time)
{
    thread_t* cur = thread_list;

    *sleep_time = 0xFFFFFFFF;

    while (cur != NULL)
    {
        // Check if the thread is running
        if (cur->state & THREAD_STATE_RUNNING)
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

                        int result = twaker->poll(cur->waker);

                        // Check if the waker is ready
                        if (result == 0)
                        {
                            cur->state &= ~THREAD_STATE_WAIT_ON_WAKER;
                            cur->waker = NULL;
                            return cur;
                        }
                        // Check if the waker has timed out
                        else if (time_cmp(twaker->start_time, time_get()) > twaker->timeout)
                        {
                            cur->state &= ~THREAD_STATE_WAIT_ON_WAKER;
                            cur->state |= THREAD_STATE_WAKER_TIMEOUT;
                            cur->waker = NULL;
                            return cur;
                        }
                        // Update the sleep time with a new minimum
                        else if (*sleep_time == 0 || time_cmp(twaker->start_time, time_get()) < *sleep_time)
                        {
                            *sleep_time = time_cmp(twaker->start_time, time_get());
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

void reschedule()
{
    uint32_t sleep_time = 0xFFFFFFFF;
    thread_t* next_thread = determine_next_thread(&sleep_time);

    if (next_thread != NULL)
    {
        thread_switch(next_thread);
    }
    else
    {
        // Start a timer and enter the idle thread.
        start_timer(sleep_time);
        thread_switch(idle_thread);
    }
}

void register_waker(void *waker)
{
    thread_t *current = thread_current();

    if (current == NULL)
    {
        return;
    }

    current->waker = waker;
}

void unregister_waker()
{
    thread_t *current = thread_current();

    if (current == NULL)
    {
        return;
    }

    current->waker = NULL;
}

void yield()
{
    thread_t *current = thread_current();

    if (current == NULL)
    {
        return;
    }

    current->stack = get_stack_pointer();

    jump_to_supervisor(SUPERVISOR_FUNC_RESCHEDULE);
}

void register_thread(void *thread)
{
    thread_t *new_thread = (thread_t *)thread;

    thread_t* cur = thread_list;
    thread_t* prev = NULL;

    // Find the correct position to insert the new thread
    while (cur != NULL)
    {
        if (cur->priority < new_thread->priority)
        {
            break;
        }

        prev = cur;
        cur = cur->next;
    }

    // Insert a new thread at the beginning of the list
    if (prev == NULL)
    {
        new_thread->next = thread_list;
        thread_list = new_thread;
    }
    // Insert a new thread somewhere in the middle or at the end of the list
    else
    {
        prev->next = new_thread;
        new_thread->next = cur;
    }
}