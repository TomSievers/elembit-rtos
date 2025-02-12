#include "message_box.h"
#include <stddef.h>

void message_box_init(message_box_t *box)
{
    semaphore_init(&box->semaphore, 0);
    box->head = NULL;
}

void message_box_post(message_box_t *box, message_t *message)
{
    
}

message_t *message_box_fetch(message_box_t *box, uint32_t timeout)
{
    return NULL;
}