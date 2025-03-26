#include "thread.h"
#include "scheduler.h"
#include "port.h"
#include "waker.h"
#include "errno.h"
#include <stddef.h>

thread_t *thread_create(thread_entry_t entry, void *arg, uint32_t priority, void *stack, uint32_t stack_size)
{
    if (stack == NULL || stack_size == 0)
    {
        errno = EINVAL;
        return NULL;
    }

    thread_t *thread = (thread_t *)stack;

    thread->state = 0;
    thread->priority = priority;
    thread->stack_size = stack_size;
    thread->stack = stack + sizeof(thread_t);
    thread->stack_start = stack + sizeof(thread_t);
    thread->arg = arg;
    thread->entry = entry;
    thread->waker = NULL;
    thread->next = NULL;

#ifdef ROUND_ROBIN
    thread->consumed_time_slice = 0;
    thread->next_in_schedule = NULL;
    thread->prev_in_schedule = NULL;
#endif

#ifdef MP
    thread->affinity = -1;
    thread->spinlock = mp_acquire_spinlock(false);

    // If we are unable to acquire a spinlock, we cannot create the thread
    if (thread->spinlock == NULL)
    {
        return NULL;
    }
#endif

    register_thread(thread);

    return thread;
}

#ifdef MP

thread_t *thread_create_on_core(uint16_t core_id, thread_entry_t entry, void *arg, uint32_t priority, void *stack, uint32_t stack_size)
{
    thread_t* thread = thread_create(entry, arg, priority, stack, stack_size);
    if (thread == NULL)
    {
        return NULL;
    }
    thread->affinity = core_id;

    return thread;
}

#endif // MP

thread_t *thread_current()
{
    return (thread_t *)get_thread_pointer();
}

void thread_sleep(uint32_t timeout)
{
    if (timeout == 0)
    {
        return;
    }

    thread_t *current = thread_current();

    if (current == NULL)
    {
        return;
    }

    timed_waker_t waker;
    init_timed_waker(&waker, NULL, NULL, timeout);

    register_waker(&waker);

    yield();

    unregister_waker();
}

int poll_joinable(void * context)
{
    thread_t *thread = (thread_t *)context;

    if (thread == NULL)
    {
        return 0;
    }

    if (thread->state & THREAD_STATE_JOINABLE)
    {
        return 0;
    }

    return 1;
}

int thread_join(thread_t *thread, uint32_t timeout)
{
    if (thread == NULL)
    {
        // Invalid thread
        errno = EINVAL;
        return -1;
    }

    if (thread->state & THREAD_STATE_JOINABLE)
    {
        return 0;
    }

    if (timeout == 0)
    {
        errno = ETIMEDOUT;
        return -1;
    }

    timed_waker_t waker;

    init_timed_waker(&waker, poll_joinable, thread, timeout);

    register_waker(&waker);

    yield();

    if (thread->state & THREAD_STATE_JOINABLE)
    {
        return 0;
    }

    errno = ETIMEDOUT;
    return -1;
}