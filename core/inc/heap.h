#pragma once

#include "mutex.h"
#include <stddef.h>

typedef struct heap
{
    mutex_t mutex;
    void *memory;
    size_t size;
} heap_t;

/**
 * @brief Initialize a heap
 * @param heap Pointer to the heap structure
 * @param memory Pointer to the memory to use for the heap
 * @param size Size of the memory
 * @return 0 on success, -1 on error and errno is set
 */
int heap_init(heap_t *heap, void *memory, uint32_t size);

/**
 * @brief Allocate memory from the heap
 * @param heap Pointer to the heap structure
 * @param size Size of the memory to allocate
 * @return Pointer to the allocated memory, NULL on error and errno is set
 * @note The function does not wait for the mutex to be available, if the mutex is not available the function returns NULL
 */
void *heap_alloc(heap_t *heap, uint32_t size);

/**
 * @brief Free memory allocated from the heap
 * @param heap Pointer to the heap structure
 * @param ptr Pointer to the memory to free
 * @note The pointer must have been allocated from this heap to avoid undefined behavior, also when freeing the function waits indefinitely for the mutex
 */
void heap_free(heap_t *heap, void *ptr);
