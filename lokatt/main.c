#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <ncurses.h>

#include "lokatt/msg.h"

// curses

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static void print(const char *fmt, ...)
{
        va_list ap;
        va_start(ap, fmt);
        pthread_mutex_lock(&lock);
        vw_printw(stdscr, fmt, ap);
        wrefresh(stdscr);
        pthread_mutex_unlock(&lock);
        va_end(ap);
}

// thread: background job

static void thread_background_cleanup(void *not_used __attribute__((unused)))
{
        print("thread 0x%08lx: cleanup\n", pthread_self());
}

static void *thread_background_main(void *arg)
{
        msg_queue_t msg_queue = (msg_queue_t)arg;
        pthread_cleanup_push(thread_background_cleanup, NULL);
        print("thread 0x%08lx: start\n", pthread_self());
        for (;;) {
                print("thread 0x%08lx: ...\n", pthread_self());
                struct msg msg;
                msg.type = MSG_TYPE_BAR;
                strcpy(msg.bar.payload, "bar");
                msg_queue_send(msg_queue, &msg);
                struct timespec t;
                t.tv_sec = 0;
                t.tv_nsec = 100000000;
                nanosleep(&t, NULL);
        }
        print("thread 0x%08lx: end\n", pthread_self());
        pthread_cleanup_pop(1);
        pthread_exit((void *)0);
}

// thread: ncurses input

static void thread_input_cleanup(void *not_used __attribute__((unused)))
{
        print("thread 0x%08lx: cleanup\n", pthread_self());
}

static void *thread_input_main(void *arg)
{
        msg_queue_t msg_queue = (msg_queue_t)arg;
        pthread_cleanup_push(thread_input_cleanup, NULL);
        print("thread 0x%08lx: start\n", pthread_self());
        for (;;) {
                struct msg msg;
                msg.type = MSG_TYPE_FOO;
                msg.foo.payload = getch();
                print("thread 0x%08lx: read %d\n", pthread_self(),
                      msg.foo.payload);
                msg_queue_send(msg_queue, &msg);
        }
        print("thread 0x%08lx: end\n", pthread_self());
        pthread_cleanup_pop(1);
        pthread_exit((void *)0);
}

// main

int main()
{
        msg_queue_t msg_queue = msg_queue_create(16);
        initscr();
        scrollok(stdscr, TRUE);
        print("main: start\n");

        pthread_t thread;
        if (pthread_create(&thread, NULL, thread_background_main,
                           (void *)msg_queue) != 0) {
                abort();
        }

        pthread_t thread2;
        if (pthread_create(&thread2, NULL, thread_input_main,
                           (void *)msg_queue) != 0) {
                abort();
        }

        for (;;) {
                struct msg msg;
                msg_queue_receive(msg_queue, &msg);
                switch (msg.type) {
                case MSG_TYPE_FOO:
                        print("main: %d: %d\n", msg.type, msg.foo.payload);
                        break;
                case MSG_TYPE_BAR:
                        print("main: %d: %s\n", msg.type, msg.bar.payload);
                        break;
                }
                if (msg.type == MSG_TYPE_FOO && msg.foo.payload == 'q') {
                        break;
                }
        }

        print("main: shut down threads\n");
        pthread_cancel(thread2);
        pthread_cancel(thread);
        pthread_join(thread2, NULL);
        pthread_join(thread, NULL);

        print("main: end (press any key to exit curses)\n");
        getch();
        endwin();
        msg_queue_destroy(msg_queue);

        return 0;
}
