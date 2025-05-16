#include "scheduler.h"
#include "thread.h"
#include "waker.h"
#include "time.h"
#include "port.h"
#include <stddef.h>
#include <stdbool.h>

#if RR_SCHEDULE

#ifndef TIME_SLICE
#define TIME_SLICE 10
#endif

static volatile thread_t *schedule;

#ifdef MULTI_PROCESSING
static volatile void *scheduler_spinlock;
#endif

extern thread_t *is_thread_runnable(volatile thread_t *cur, uint32_t *sleep_time);

static inline void scheduler_lock();
static inline void scheduler_unlock();

static void remove_thread_from_schedule(volatile thread_t *thread)
{
    scheduler_lock();

    // Remove the thread from the schedule
    if (thread->prev_in_schedule != NULL)
    {
        thread->prev_in_schedule->next_in_schedule = thread->next_in_schedule;
    }
    else
    {
        schedule = thread->next_in_schedule;
    }
    if (thread->next_in_schedule != NULL)
    {
        thread->next_in_schedule->prev_in_schedule = thread->prev_in_schedule;
    }
    thread->next_in_schedule = NULL;
    thread->prev_in_schedule = NULL;

    scheduler_unlock();
}

static void initialize_thread_schedule(volatile thread_t *thread_list)
{
    volatile thread_t *cur = thread_list;

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

volatile thread_t *determine_next_thread(volatile thread_t *thread_list, uint32_t *sleep_time)
{
    // Get the last thread that was run before the scheduler was called, update the time slice for this thread.
    volatile thread_t *last_thread = get_thread_pointer();

    if (last_thread != NULL)
    {
        lock_thread(last_thread);

        last_thread->state &= ~THREAD_STATE_RUNNING;
        last_thread->consumed_time_slice += time_get() - last_thread->time_slice_start;

        unlock_thread(last_thread);
    }

    volatile thread_t *cur = schedule;

    // The schedule is empty, start from the beginning
    scheduler_lock();
    if (schedule == NULL)
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
#ifdef MULTI_PROCESSING
            if (cur->affinity == -1 || cur->affinity == core_id())
#endif
            {
                lock_thread(cur);

                volatile thread_t *ret = is_thread_runnable(cur, sleep_time);

                if (ret != NULL)
                {
                    // Indicate to possibly another core that we are running this thread
                    ret->state |= THREAD_STATE_RUNNING;
                    ret->time_slice_start = time_get();
                    schedule_time_slice_end(TIME_SLICE - ret->consumed_time_slice);
                }

                unlock_thread(cur);

                if (ret != NULL)
                {
                    return ret;
                }
            }
        }

        cur = cur->next_in_schedule;
    }

    // There are no threads to run, restart the schedule.
    scheduler_lock();
    schedule = thread_list;
    initialize_thread_schedule(schedule);
    scheduler_unlock();

    return NULL;
}

#ifdef MULTI_PROCESSING
static inline void scheduler_lock()
{
    mp_spinlock_lock(scheduler_spinlock);
}
static inline void scheduler_unlock()
{
    mp_spinlock_unlock(scheduler_spinlock);
}
#else
static inline void scheduler_lock()
{
}
static inline void scheduler_unlock()
{
}
#endif

#endif
