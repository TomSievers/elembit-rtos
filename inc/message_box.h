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

/**
 * @brief Initialize a message box
 * @param box Pointer to the message box structure
 * @param heap Pointer to the memory to use for the heap
 * @param heap_size Size of the memory
 * @return 0 on success, -1 on error and errno is set
 */
int message_box_init(message_box_t *box, void* heap, uint32_t heap_size);

/**
 * @brief Post a message to the message box
 * @param box Pointer to the message box structure
 * @param data Pointer to the data to post
 * @param length Length of the data
 * @param timeout Timeout in milliseconds
 * @return 0 on success, -1 on error and errno is set
 */
int message_box_post(message_box_t *box, uint8_t* data, uint32_t length, uint32_t timeout);

/**
 * @brief Fetch a message from the message box
 * @param box Pointer to the message box structure
 * @param data Pointer to the buffer to store the message
 * @param timeout Timeout in milliseconds
 * @return number of bytes on succes or 0 if zero length message, -1 on error and errno is set
 */
int message_box_fetch_buf(message_box_t *box, uint8_t* data, uint32_t timeout);
