#ifndef LOKATT_MSG_H
#define LOKATT_MSG_H
#include <stddef.h>
#include <stdint.h>

/**
 * Thread safe, single consumer, multiple producer message queue.
 */

struct msg_foo {
        int payload;
};

struct msg_bar {
        char payload[10];
};

struct msg {
        enum { MSG_TYPE_FOO,
               MSG_TYPE_BAR,
        } type;
        union {
                struct msg_foo foo;
                struct msg_bar bar;
        };
};

typedef uintptr_t msg_queue_t;

/**
 * Create a new message queue with room for size number of messages. The caller
 * becomes the owner of the queue and is responsible for calling
 * msg_queue_destroy when done.
 *
 * Not thread-safe.
 */
msg_queue_t msg_queue_create(size_t size);

/**
 * Destroy the message queue.
 *
 * Not thread-safe.
 */
void msg_queue_destroy(msg_queue_t queue);

/**
 * Post a new message to the end of the queue. The contents of msg is copied so
 * the caller remains the owner of msg.
 *
 * If the queue is full, block until room becomes available.
 *
 * Thread safe.
 */
void msg_queue_send(msg_queue_t queue, const struct msg *msg);

/**
 * Get a message from the front of the queue.
 *
 * If the queue is empty, block until a message becomes
 * available.
 *
 * Thread safe.
 */
void msg_queue_receive(msg_queue_t queue, struct msg *msg);

#endif
