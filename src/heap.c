#include <heap.h>
#include <stddef.h>

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

void heap_init(heap_t *heap, void *memory, uint32_t size)
{
    mutex_init(&heap->mutex);
    heap->memory = memory;
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

                        return (void *)(header + 1);
                    }
                }
                header = header->next;
            }
        }
    }

    return NULL;
}

void heap_free(heap_t *heap, void *ptr)
{
    if (ptr != NULL)
    {
        int ret = mutex_lock(&heap->mutex, 0);

        if (ret == 0)
        {
            element_header_t *header = (element_header_t *)(((intptr_t)ptr) - sizeof(element_header_t));
            clear_allocated(header);

            // Check if the next block is also free, combine the blocks
            while (header->next != NULL && !is_allocated(header->next))
            {
                header->next = header->next->next;
            }
        }
    }
}



