#include "mutex.h"
#include "port.h"

void mutex_init(mutex_t *mutex)
{
    semaphore_init(mutex, 1);
}

int mutex_lock(mutex_t *mutex, uint32_t timeout)
{
    return semaphore_wait(mutex, timeout);
}

void mutex_unlock(mutex_t *mutex)
{
    uint32_t state = enter_critical_section();

    // Only signal the semaphore if it is not already signaled, this is to prevent multiple unlocking
    if (*mutex == 0)
    {
        semaphore_signal(mutex);
    }

    exit_critical_section(state);
}