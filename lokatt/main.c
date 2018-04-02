#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <ncurses.h> // TODO: move calls to ncurses to window manager

#include "lokatt/buffer.h"
#include "lokatt/error.h"
#include "lokatt/msg.h"
#include "lokatt/window.h"

// thread: background job

static char *lorem_ipsum[] = {
    "1 Lorem ipsum dolor sit amet, consectetur adipiscing elit.",
    "2 Morbi interdum fringilla quam ut imperdiet.",
    "3 Etiam non pretium erat, a sagittis est.",
    "4 Ut mattis turpis bibendum lectus elementum,",
    "5 eu dictum quam dapibus.Donec nec risus consectetur,",
    "6 pharetra dui at,",
    "7 sodales augue.",
    "8 Integer ac justo tempor,",
    "9 faucibus purus et lobortis tortor.",
    "10 Aenean egestas tempor neque,",
    "11 sed mollis augue rutrum in.",
    "12 Aliquam nec dapibus dui.",
};

static void thread_background_cleanup(void *not_used __attribute__((unused)))
{
}

static void *thread_background_main(void *arg)
{
        const size_t N = sizeof(lorem_ipsum) / sizeof(char *);
        msg_queue_t msg_queue = (msg_queue_t)arg;
        pthread_cleanup_push(thread_background_cleanup, NULL);
        for (;;) {
                struct msg msg;
                msg.type = MSG_TYPE_BAR;
                strcpy(msg.bar.payload, lorem_ipsum[rand() % N]);
                msg_queue_send(msg_queue, &msg);
                struct timespec t;
                t.tv_sec = 0;
                t.tv_nsec = 100000000;
                nanosleep(&t, NULL);
        }
        pthread_cleanup_pop(1);
        pthread_exit((void *)0);
}

// thread: ncurses input

static void thread_input_cleanup(void *not_used __attribute__((unused)))
{
}

static void *thread_input_main(void *arg)
{
        msg_queue_t msg_queue = (msg_queue_t)arg;
        pthread_cleanup_push(thread_input_cleanup, NULL);
        for (;;) {
                struct msg msg;
                msg.type = MSG_TYPE_FOO;
                msg.foo.payload = getch();
                msg_queue_send(msg_queue, &msg);
        }
        pthread_cleanup_pop(1);
        pthread_exit((void *)0);
}

// window manager

static void create_windows(window_t *main_window, window_t *status_bar)
{
        *main_window = window_create(0, 0, 0, LINES - 1);
        *status_bar = window_create(0, LINES - 1, 0, 1);
}

static void destroy_windows(window_t main_window, window_t status_bar)
{
        window_destroy(main_window);
        window_destroy(status_bar);
}

// main

int main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
        srand(time(NULL));
        msg_queue_t msg_queue = msg_queue_create(16);
        initscr();
        curs_set(0);
        cbreak();
        noecho();

        buffer_t main_buffer = buffer_create();
        buffer_t status_bar_buffer = buffer_create();

        window_t main_window, status_bar;
        create_windows(&main_window, &status_bar);
        window_set_buffer(main_window, main_buffer);
        window_set_buffer(status_bar, status_bar_buffer);

        pthread_t thread;
        if (pthread_create(&thread, NULL, thread_background_main,
                           (void *)msg_queue) != 0) {
                die("pthread_create");
        }

        pthread_t thread2;
        if (pthread_create(&thread2, NULL, thread_input_main,
                           (void *)msg_queue) != 0) {
                die("pthread_create");
        }

        for (;;) {
                struct msg msg;
                msg_queue_receive(msg_queue, &msg);
                switch (msg.type) {
                case MSG_TYPE_FOO:
                        break;
                case MSG_TYPE_BAR:
                        buffer_add_line(main_buffer, msg.bar.payload,
                                        strlen(msg.bar.payload));

                        char buf[1024];
                        snprintf(buf, 1024, "%zu line(s)",
                                 buffer_size(main_buffer));
                        buffer_clear(status_bar_buffer);
                        buffer_add_line(status_bar_buffer, buf, strlen(buf));

                        window_refresh(main_window);
                        window_refresh(status_bar);

                        doupdate();
                        break;
                }
                if (msg.type == MSG_TYPE_FOO && msg.foo.payload == 'q') {
                        break;
                }
        }

        pthread_cancel(thread2);
        pthread_cancel(thread);
        pthread_join(thread2, NULL);
        pthread_join(thread, NULL);

        destroy_windows(main_window, status_bar);
        endwin();

        buffer_destroy(status_bar_buffer);
        buffer_destroy(main_buffer);
        msg_queue_destroy(msg_queue);

        return 0;
}
