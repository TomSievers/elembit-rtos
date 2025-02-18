#include "message_box.h"
#include "port.h"
#include <stddef.h>

void message_box_init(message_box_t *box, void* heap, uint32_t heap_size)
{
    mutex_init(&box->mutex);
    heap_init(&box->heap, heap, heap_size);
    box->head = NULL;
}

int message_box_post(message_box_t *box, uint8_t* data, uint32_t length, uint32_t timeout)
{
    // Check if data is valid
    if (data == NULL || length == 0)
    {
        return -1;
    }

    // Allocate memory for the message struct
    message_t *message = heap_alloc(&box->heap, sizeof(message_t));

    if (message == NULL)
    {
        mutex_unlock(&box->mutex);
        return -1;
    }

    // Allocate memory for the message data
    message->next = NULL;
    message->length = length;
    message->data = heap_alloc(&box->heap, length);

    if (message->data == NULL)
    {
        heap_free(&box->heap, message);
        return -1;
    }

    // Copy the data to the message, do this before locking the mutex to 
    // avoid locking the mutex for longer than necessary.
    for (uint32_t i = 0; i < length; i++)
    {
        message->data[i] = data[i];
    }

    int res = mutex_lock(&box->mutex, timeout);

    if (res != 0)
    {
        heap_free(&box->heap, message->data);
        heap_free(&box->heap, message);
        return res;
    }

    if (box->head == NULL)
    {
        box->head = message;
        box->tail = message;
    }
    else
    {
        box->tail->next = message;
        box->tail = message;
    }

    mutex_unlock(&box->mutex);

    return 0;
}

int message_box_fetch_buf(message_box_t *box, uint8_t* data, uint32_t timeout)
{
    if (data == NULL)
    {
        return -1;
    }

    // Check if there are any messages in the box, if not return -1
    int state = enter_critical_section();

    if (box->head == NULL)
    {
        exit_critical_section(state);
        return -1;
    }

    exit_critical_section(state);

    // Lock the mutex to access the message box
    int res = mutex_lock(&box->mutex, timeout);

    if (res != 0)
    {
        return res;
    }

    message_t *message = box->head;

    box->head = message->next;

    mutex_unlock(&box->mutex);

    for (uint32_t i = 0; i < message->length; i++)
    {
        data[i] = message->data[i];
    }

    // Clean up the message
    heap_free(&box->heap, message->data);
    heap_free(&box->heap, message);

    return 0;
}

int message_box_free(message_box_t *box, uint8_t* data)
{
    return 0;
}