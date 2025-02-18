#pragma once

#include "mutex.h"
#include <stddef.h>

typedef struct heap
{
    mutex_t mutex;
    void *memory;
    size_t size;
} heap_t;

void heap_init(heap_t *heap, void *memory, uint32_t size);

void *heap_alloc(heap_t *heap, uint32_t size);

void heap_free(heap_t *heap, void *ptr);