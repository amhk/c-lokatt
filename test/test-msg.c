#include <pthread.h>
#include <string.h>
#include <time.h>

#include "lokatt/error.h"
#include "lokatt/msg.h"

#include "test.h"

TEST(msg, basic_send_receive)
{
        struct msg msg1;
        msg1.type = MSG_TYPE_FOO;
        msg1.foo.payload = 1234;

        struct msg msg2;
        msg2.type = MSG_TYPE_BAR;
        strcpy(msg2.bar.payload, "test");

        msg_queue_t queue = msg_queue_create(16);

        msg_queue_send(queue, &msg1);
        msg_queue_send(queue, &msg2);

        struct msg msg;
        msg_queue_receive(queue, &msg);
        ASSERT_EQ(msg.type, MSG_TYPE_FOO);
        ASSERT_EQ(msg.foo.payload, 1234);

        msg_queue_receive(queue, &msg);
        ASSERT_EQ(msg.type, MSG_TYPE_BAR);
        ASSERT_EQ(strcmp(msg.bar.payload, "test"), 0);

        msg_queue_destroy(queue);
}

TEST(msg, loop_around_queue_size)
{
        msg_queue_t queue = msg_queue_create(3);

        for (int i = 0; i < 10; i++) {
                struct msg msg;
                msg.type = MSG_TYPE_FOO;
                msg.foo.payload = i;
                msg_queue_send(queue, &msg);

                memset(&msg, 0, sizeof(struct msg));
                msg_queue_receive(queue, &msg);
                ASSERT_EQ(msg.type, MSG_TYPE_FOO);
                ASSERT_EQ(msg.foo.payload, i);
        }

        msg_queue_destroy(queue);
}

struct args {
        msg_queue_t queue;
        size_t iterations;
        long delay_ns;
};

static void *producer_thread(void *args_)
{
        struct args *args = (struct args *)args_;
        for (size_t i = 0; i < args->iterations; i++) {
                struct msg msg;
                msg.type = MSG_TYPE_FOO;
                msg.foo.payload = i;
                msg_queue_send(args->queue, &msg);

                if (args->delay_ns > 0) {
                        struct timespec t;
                        t.tv_sec = 0;
                        t.tv_nsec = args->delay_ns;
                        nanosleep(&t, NULL);
                }
        }
        pthread_exit((void *)0);
}

TEST(msg, consumer_faster_than_producer)
{
        const size_t N = 17;
        msg_queue_t queue = msg_queue_create(3);

        struct args args = {queue, N, 10};
        pthread_t thread;
        if (pthread_create(&thread, NULL, producer_thread, &args)) {
                die("pthread_create");
        }

        for (size_t i = 0; i < N; i++) {
                struct msg msg;
                msg_queue_receive(queue, &msg);
                ASSERT_EQ(msg.type, MSG_TYPE_FOO);
                ASSERT_EQ(msg.foo.payload, (int)i);
        }

        pthread_join(thread, NULL);
        msg_queue_destroy(queue);
}

TEST(msg, producer_faster_than_consumer)
{
        const size_t N = 17;
        msg_queue_t queue = msg_queue_create(3);

        struct args args = {queue, N, 0};
        pthread_t thread;
        if (pthread_create(&thread, NULL, producer_thread, &args)) {
                die("pthread_create");
        }

        for (size_t i = 0; i < N; i++) {
                struct msg msg;
                msg_queue_receive(queue, &msg);
                ASSERT_EQ(msg.type, MSG_TYPE_FOO);
                ASSERT_EQ(msg.foo.payload, (int)i);

                struct timespec t;
                t.tv_sec = 0;
                t.tv_nsec = 10;
                nanosleep(&t, NULL);
        }

        pthread_join(thread, NULL);
        msg_queue_destroy(queue);
}
