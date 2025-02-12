#include "semaphore.h"
#include "scheduler.h"
#include "waker.h"
#include "thread.h"
#include "port.h"

void semaphore_init(semaphore_t *sem, uint32_t count)
{
    *sem = count;
}

int semaphore_poll(void *waker)
{
    timed_waker_t *tw = (timed_waker_t *)waker;

    uint32_t* sem = tw->context;

    if (*sem > 0)
    {
        *sem -= 1;
        return 1;
    }

    return 0;
}

int semaphore_wait(semaphore_t *sem, uint32_t timeout)
{
    uint32_t state = enter_critical_section();

    // If the semaphore is greater than 0, decrement it and return 1
    if (*sem > 0)
    {
        *sem -= 1;
        exit_critical_section(state);
        return 0;
    }

    exit_critical_section(state);

    // If the timeout is 0, return immediately
    if (timeout == 0)
    {
        return -1;
    }

    // If the semaphore is 0, we need to wait for it to be signaled
    // Initialize a timed waker and register it
    timed_waker_t waker;
    init_timed_waker(&waker, semaphore_poll, sem, timeout);
    register_waker(&waker);

    // Yield the current thread
    yield();
    // Unregister the waker after we are woken up
    unregister_waker();

    // Check if we timed out
    thread_t *current = thread_current();
    if (current->state & THREAD_STATE_WAKER_TIMEOUT)
    {
        return -1;
    }

    return 0;
}

void semaphore_signal(semaphore_t *sem)
{
    semaphore_signal_from_isr(sem);

    yield();
}

void semaphore_signal_from_isr(semaphore_t *sem)
{
    uint32_t state = enter_critical_section();

    *sem += 1;

    exit_critical_section(state);
}

