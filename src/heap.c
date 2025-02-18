#include <heap.h>
#include <stddef.h>
#include <errno.h>

#include "time.h"

inline int is_allocated(void *ptr)
{
    return ((intptr_t)ptr) & 0x01;
}

inline void set_allocated(void *ptr)
{
    ptr = (void *)(((intptr_t)ptr) | 0x01);
}

inline void clear_allocated(void *ptr)
{
    ptr = (void *)(((intptr_t)ptr) & ~0x01);
}

inline void* get_next(void *ptr)
{
    return (void *)(((intptr_t)ptr) & ~0x01);
}

typedef struct element_header
{
    struct element_header *next;
} element_header_t;

int heap_init(heap_t *heap, void *memory, uint32_t size)
{
    if (memory == NULL || size == 0)
    {
        errno = EINVAL;
        return -1;
    }

    mutex_init(&heap->mutex);
    heap->memory = memory;
    heap->size = size;

    return 0;
}

void *heap_alloc(heap_t *heap, uint32_t size)
{
    void *ptr = NULL;
    if (size > 0)
    {
        int ret = mutex_lock(&heap->mutex, 0);

        if (ret == 0)
        {
            element_header_t *header = (element_header_t *)heap->memory;

            while (header != NULL)
            {
                if (!is_allocated(header))
                {
                    // Check if the next block is also free, combine the blocks
                    while (header->next != NULL && !is_allocated(header->next))
                    {
                        header->next = header->next->next;
                    }

                    uint32_t block_size = 0;

                    // Calculate the size of the block of this element
                    if (header->next != NULL)
                    {
                        block_size = (intptr_t)header->next - (intptr_t)header;
                    }
                    else
                    {
                        block_size = ((intptr_t)header) - ((intptr_t)heap->memory) - heap->size;
                    }

                    if (block_size >= size + sizeof(element_header_t))
                    {
                        element_header_t *new_header = (element_header_t *)(((intptr_t)header) + size + sizeof(element_header_t));

                        new_header->next = header->next;
                        header->next = new_header;
                        set_allocated(header);
                        
                        mutex_unlock(&heap->mutex);

                        return (void *)(header + 1);
                    }
                }
                header = get_next(header->next);
            }

            mutex_unlock(&heap->mutex);

            errno = ENOMEM;
        }
        else
        {
            errno = ETIMEDOUT;
        }
    }

    return NULL;
}

void heap_free(heap_t *heap, void *ptr)
{
    errno = 0;

    if (ptr != NULL)
    {
        int ret = mutex_lock(&heap->mutex, TIMEOUT_INF);

        if (ret == 0)
        {
            element_header_t *header = (element_header_t *)(((intptr_t)ptr) - sizeof(element_header_t));
            clear_allocated(header);

            // Check if the next block is also free, combine the blocks
            while (header->next != NULL && !is_allocated(header->next))
            {
                header->next = header->next->next;
            }

            mutex_unlock(&heap->mutex);

            return;
        }
        else
        {
            errno = ETIMEDOUT;
        }
    }

    errno = EINVAL;
}



