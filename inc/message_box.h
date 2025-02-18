#pragma once

#include <stdint.h>
#include "heap.h"
#include "mutex.h"

typedef struct message
{
    uint32_t length;
    struct message *next;
    uint8_t *data;
} message_t;

typedef struct message_box
{
    heap_t heap;
    mutex_t mutex;
    message_t *head;
    message_t *tail;
} message_box_t;

void message_box_init(message_box_t *box, void* heap, uint32_t heap_size);

int message_box_post(message_box_t *box, uint8_t* data, uint32_t length, uint32_t timeout);

int message_box_fetch_buf(message_box_t *box, uint8_t* data, uint32_t timeout);
