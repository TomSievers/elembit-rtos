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

    register_thread(thread);

    return thread;
}

int thread_run(thread_t *thread, thread_entry_t entry, void *arg)
{
    if (thread == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    thread->state = 0;
    thread->entry = entry;
    thread->arg = arg;
    thread->stack = thread->stack_start;

    return 0;
}

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

int thread_join_reusable(thread_t *thread, uint32_t timeout)
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

int thread_join(thread_t *thread, uint32_t timeout)
{
    int res = thread_join_reusable(thread, timeout);

    // Thread was joined, unregister it
    if (res == 0)
    {
        unregister_thread(thread);
    }

    return res;
}