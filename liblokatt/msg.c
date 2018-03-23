#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "lokatt/error.h"
#include "lokatt/msg.h"

struct msg_queue {
        size_t size, read_head, write_head;
        pthread_mutex_t mutex;
        pthread_cond_t cond;
        struct msg queue[0];
};

msg_queue_t msg_queue_create(size_t size)
{
        if (size < 2) {
                die("invalid argument");
        }
        struct msg_queue *queue =
            malloc(sizeof(struct msg_queue) + size * sizeof(struct msg));
        if (!queue) {
                die("malloc");
        }
        if (pthread_mutex_init(&queue->mutex, NULL) != 0) {
                die("pthread_mutex_init");
        }
        if (pthread_cond_init(&queue->cond, NULL) != 0) {
                die("pthread_cond_init");
        }
        queue->size = size;
        queue->read_head = 0;
        queue->write_head = 0;
        return (msg_queue_t)queue;
}

static size_t advance_head(size_t head, size_t size)
{
        return (head + 1) % size;
}

static int can_write(const struct msg_queue *q)
{
        return advance_head(q->write_head, q->size) != q->read_head;
}

static int can_read(const struct msg_queue *q)
{
        return q->read_head != q->write_head;
}

void msg_queue_destroy(msg_queue_t queue)
{
        struct msg_queue *q = (struct msg_queue *)queue;
        if (pthread_cond_destroy(&q->cond) != 0) {
                die("pthread_cond_destroy");
        }
        if (pthread_mutex_destroy(&q->mutex) != 0) {
                die("pthread_mutex_destroy");
        }
        free(q);
}

void msg_queue_send(msg_queue_t queue, const struct msg *msg)
{
        struct msg_queue *q = (struct msg_queue *)queue;

        if (pthread_mutex_lock(&q->mutex) != 0) {
                die("pthread_mutex_lock");
        }
        if (!can_write(q)) {
                if (pthread_cond_wait(&q->cond, &q->mutex) != 0) {
                        die("pthread_cond_wait");
                }
        }
        memcpy(&q->queue[q->write_head], msg, sizeof(*msg));
        q->write_head = advance_head(q->write_head, q->size);
        pthread_cond_signal(&q->cond);
        if (pthread_mutex_unlock(&q->mutex) != 0) {
                die("pthread_mutex_unlock");
        }
}

void msg_queue_receive(msg_queue_t queue, struct msg *msg)
{
        struct msg_queue *q = (struct msg_queue *)queue;

        if (pthread_mutex_lock(&q->mutex) != 0) {
                die("pthread_mutex_lock");
        }
        if (!can_read(q)) {
                if (pthread_cond_wait(&q->cond, &q->mutex) != 0) {
                        die("pthread_cond_wait");
                }
        }
        memcpy(msg, &q->queue[q->read_head], sizeof(*msg));
        q->read_head = advance_head(q->read_head, q->size);
        pthread_cond_signal(&q->cond);
        if (pthread_mutex_unlock(&q->mutex) != 0) {
                die("pthread_mutex_unlock");
        }
}
