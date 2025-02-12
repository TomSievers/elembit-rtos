#pragma once

#include <stdint.h>
#include "semaphore.h"

typedef struct message
{
    uint32_t length;
    uint8_t *data;
    struct message *next;
} message_t;

typedef struct message_box
{
    semaphore_t semaphore;
    message_t *head;
} message_box_t;

void message_box_init(message_box_t *box);

void message_box_post(message_box_t *box, message_t *message);

message_t *message_box_fetch(message_box_t *box, uint32_t timeout);