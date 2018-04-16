#include <fcntl.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <ncurses.h> // TODO: move calls to ncurses to window manager

#include "lokatt/buffer.h"
#include "lokatt/error.h"
#include "lokatt/msg.h"
#include "lokatt/window.h"

// thread: background job

struct thread_background_args {
        msg_queue_t msg_queue;
        char path[128];
};

static void thread_background_cleanup(void *arg)
{
        int *fd = (int *)arg;
        if (*fd != -1) {
                close(*fd);
        }
}

static void thread_background_loop(msg_queue_t msg_queue, int fd)
{
        struct msg msg;
        msg.type = MSG_TYPE_LOGGER_ENTRY;
        memset(&msg.logger_entry, 0, LOGGER_ENTRY_MAX_LEN);
        while (adb_next_entry(fd, &msg.logger_entry) != -1) {
                msg_queue_send(msg_queue, &msg);
                struct timespec t;
                t.tv_sec = 0;
                t.tv_nsec = 100000000;
                nanosleep(&t, NULL);
                memset(&msg.logger_entry, 0, LOGGER_ENTRY_MAX_LEN);
        }
}

static void *thread_background_main(void *arg)
{
        const struct thread_background_args *args =
            (const struct thread_background_args *)arg;
        int fd = -1;
        if (strlen(args->path) > 0) {
                fd = open(args->path, O_RDONLY);
        }
        pthread_cleanup_push(thread_background_cleanup, &fd);
        if (fd != -1) {
                thread_background_loop(args->msg_queue, fd);
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
        initscr();
        curs_set(0);
        cbreak();
        noecho();
        werase(stdscr);
        wrefresh(stdscr);

        msg_queue_t msg_queue = msg_queue_create(16);
        repo_t repo = repo_create();
        buffer_t main_buffer = buffer_logcat_create(repo);
        buffer_t status_bar_buffer = buffer_text_create();

        window_t main_window, status_bar;
        create_windows(&main_window, &status_bar);
        window_set_buffer(main_window, main_buffer);
        window_set_buffer(status_bar, status_bar_buffer);

        struct thread_background_args args = {
            .msg_queue = msg_queue,
        };
        strncpy(args.path, argc > 1 ? argv[1] : "", sizeof(args.path));
        pthread_t thread;
        if (pthread_create(&thread, NULL, thread_background_main, &args) != 0) {
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
                case MSG_TYPE_BAR:
                        break;
                case MSG_TYPE_LOGGER_ENTRY: {
                        // update main buffer
                        const size_t gid = repo_add(repo, &msg.logger_entry);
                        const struct logcat_entry *entry = repo_peek(repo, gid);
                        (void)buffer_logcat_accept(main_buffer, gid, entry);

                        // update status bar buffer
                        char buf[1024];
                        snprintf(buf, sizeof(buf), "%zu line(s)",
                                 buffer_size(main_buffer));
                        buffer_clear(status_bar_buffer);
                        buffer_text_add(status_bar_buffer, buf, strlen(buf));

                        // update UI
                        window_refresh(main_window);
                        window_refresh(status_bar);
                        doupdate();
                } break;
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
        repo_destroy(repo);
        msg_queue_destroy(msg_queue);

        return 0;
}
