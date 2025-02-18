#include "scheduler.h"
#include "thread.h"
#include "waker.h"
#include "time.h"
#include "port.h"
#include <semaphore.h>
#include <errno.h>
#include <stddef.h>

static thread_t* thread_list = NULL;
static thread_t idle_thread;
static uint32_t last_thread_id = 0;

#define IDLE_THREAD_ID 0xFFFFFFFF

void scheduler_init()
{
    idle_thread.local_storage[0] = 0;
    idle_thread.stack = get_stack_pointer();
    idle_thread.id = IDLE_THREAD_ID;
    set_thread_pointer(&idle_thread);

}

thread_t* determine_next_thread(uint32_t* sleep_time)
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
        time_start_timer(sleep_time);
        thread_switch(&idle_thread);
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
    current->state |= THREAD_STATE_WAIT_ON_WAKER;
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
    register thread_t *current = thread_current();

    if (current == NULL)
    {
        return;
    }

    current->stack = get_stack_pointer();
    current->local_storage[0] = errno;

    jump_to_supervisor(SUPERVISOR_FUNC_RESCHEDULE);

    errno = current->local_storage[0];
}

void register_thread(void *thread)
{
    thread_t *new_thread = (thread_t *)thread;

    // Set the thread id and increment the global thread id
    new_thread->id = last_thread_id++;

    thread_t* cur = thread_list;

    // Find the correct position to insert the new thread
    while (cur != NULL)
    {
        if (cur->priority < new_thread->priority)
        {
            break;
        }

        cur = cur->next;
    }

    // Insert a new thread at the beginning of the list
    if (cur->prev == NULL)
    {
        new_thread->prev = NULL;
        new_thread->next = thread_list;
        thread_list = new_thread;
    }
    // Insert a new thread somewhere in the middle or at the end of the list
    else
    {
        new_thread->prev = cur->prev;
        new_thread->next = cur;
        new_thread->prev->next = new_thread;
        cur->prev = new_thread;
    }
}

void unregister_thread(void *thread)
{
    thread_t *cur = thread;

    cur->prev->next = cur->next;
    cur->next->prev = cur->prev;
}

void thread_entry()
{
    register thread_t *current = thread_current();

    if (current != NULL && current->entry != NULL)
    {
        // Mark the thread as running
        current->state |= THREAD_STATE_RUNNING;

        current->entry(current->arg);

        // The thread has finished executing, mark it as joinable
        current->state |= THREAD_STATE_JOINABLE;
        current->state &= ~THREAD_STATE_RUNNING;
    }

    // Make sure we never return (this could cause undefined behavior)
    while (1)
    {
        yield();
    }
}