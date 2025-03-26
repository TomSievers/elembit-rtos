#include "scheduler.h"
#include "thread.h"
#include "waker.h"
#include "time.h"
#include "port.h"
#include <stddef.h>
#include <stdbool.h>

#if defined(ROUND_ROBIN)

#ifndef TIME_SLICE
#define TIME_SLICE 10
#endif

static thread_t *schedule;

#ifdef MP
static void* scheduler_spinlock;
#endif

static inline void scheduler_lock();
static inline void scheduler_unlock();

static inline void lock_thread(thread_t *thread);
static inline void unlock_thread(thread_t *thread);

static void remove_thread_from_schedule(thread_t *thread)
{
    scheduler_lock();
    if (thread->prev_in_schedule == NULL)
    {
        schedule = thread->next_in_schedule;
    }
    else
    {
        thread->prev_in_schedule->next_in_schedule = thread->next_in_schedule;
    }

    if (thread->next_in_schedule != NULL)
    {
        thread->next_in_schedule->prev_in_schedule = thread->prev_in_schedule;
    }

    thread->prev_in_schedule = NULL;
    thread->next_in_schedule = NULL;
    scheduler_unlock();

}

static void initialize_thread_schedule(thread_t *thread_list)
{
    thread_t *cur = thread_list;

    while (cur != NULL)
    {
        cur->consumed_time_slice = 0;
        cur->next_in_schedule = cur->next;
        cur->prev_in_schedule = cur->prev;

        cur = cur->next;
    }
}

void schedule_impl_init()
{
    schedule = NULL;
    scheduler_spinlock = mp_acquire_spinlock(true);
}

thread_t *determine_next_thread(thread_t *thread_list, uint32_t *sleep_time)
{
    thread_t *cur = schedule;

    // The schedule is empty, start from the beginning
    scheduler_lock();
    if (cur == NULL)
    {
        cur = thread_list;
        schedule = cur;
        initialize_thread_schedule(schedule);
    }
    scheduler_unlock();

    *sleep_time = 0xFFFFFFFF;

    while (cur != NULL)
    {
        // The thread has consumed its time slice, remove it from the schedule
        if (cur->consumed_time_slice > TIME_SLICE)
        {
            remove_thread_from_schedule(cur);
        }
        // A joineable thread will never be run again remove it from the thread list.
        else if (cur->state & THREAD_STATE_JOINABLE)
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

                thread_t *ret = NULL;

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

                unlock_thread(cur);
            }
        }

        cur = cur->next_in_schedule;
    }

    // There are no threads to run, restart the schedule on the next time slice.
    schedule = NULL;
    return NULL;
}

#ifdef MP
static inline void scheduler_lock()
{
    mp_spinlock_lock(scheduler_spinlock);
}
static inline void scheduler_unlock()
{
    mp_spinlock_unlock(scheduler_spinlock);
}
static inline void lock_thread(thread_t *thread)
{
    mp_spinlock_lock(thread->spinlock);
}
static inline void unlock_thread(thread_t *thread)
{
    mp_spinlock_unlock(thread->spinlock);
}
#else
static inline void scheduler_lock()
{
}
static inline void scheduler_unlock()
{
}
static inline void lock_thread(thread_t *thread)
{
    (void)thread;
}
static inline void unlock_thread(thread_t *thread)
{
    (void)thread;
}
#endif

#endif