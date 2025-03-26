#include "mutex.h"
#include "port.h"
#include "thread.h"
#include <errno.h>
#include <stddef.h>

int mutex_init(mutex_t *mutex)
{
    if (mutex == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    semaphore_init(&mutex->semaphore, 1);
    mutex->thread_id = 0;

    return 0;
}

int mutex_lock(mutex_t *mutex, uint32_t timeout)
{
    int res = semaphore_wait(&mutex->semaphore, timeout);

    // Check if we acquired the lock, and we are in a thread context, then store the thread id
    if (res == 0 && get_thread_pointer() != NULL)
    {
        mutex->thread_id = ((thread_t*)get_thread_pointer())->id;
    }

    return res;
}

int mutex_unlock(mutex_t *mutex)
{
    uint32_t state = enter_critical_section();

    int res = 0;

    // Only signal the semaphore if it is not already signaled, and we are the owner
    if (mutex->semaphore == 0 && mutex->thread_id != ((thread_t*)get_thread_pointer())->id)
    {
        semaphore_signal(&mutex->semaphore);
    }
    else
    {
        // We are not the owner of the mutex
        errno = EACCES;
        res = -1;
    }

    exit_critical_section(state);

    return res;
}