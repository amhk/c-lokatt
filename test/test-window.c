#include <string.h>

#include <ncurses.h>

#include "lokatt/buffer.h"
#include "lokatt/window.h"

#include "test.h"

TEST(window, can_create_window)
{
        initscr();
        window_t w = window_create(0, 0, 80, 40);
        window_destroy(w);
        endwin();
}

TEST(window, dump)
{

        initscr();
        window_t w = window_create(0, 0, 8, 4);

        char buf[12 * 4 + 1];
        window_dump(w, buf, sizeof(buf) - 1);
        ASSERT_EQ(strcmp("        "
                         "        "
                         "        "
                         "        ",
                         buf),
                  0);

        buffer_t b = buffer_create();
        window_set_buffer(w, b);
        window_dump(w, buf, sizeof(buf) - 1);
        ASSERT_EQ(strcmp("        "
                         "        "
                         "        "
                         "        ",
                         buf),
                  0);

        buffer_add_line(b, "one", 4);
        buffer_add_line(b, "two", 4);
        window_dump(w, buf, sizeof(buf) - 1);
        ASSERT_EQ(strcmp("        "
                         "        "
                         "        "
                         "        ",
                         buf),
                  0);

        window_refresh(w);
        window_dump(w, buf, sizeof(buf) - 1);
        ASSERT_EQ(strcmp("        "
                         "        "
                         "one     "
                         "two     ",
                         buf),
                  0);

        buffer_add_line(b, "three", 6);
        buffer_add_line(b, "four", 5);
        buffer_add_line(b, "five", 5);
        window_refresh(w);
        window_dump(w, buf, sizeof(buf) - 1);
        ASSERT_EQ(strcmp("two     "
                         "three   "
                         "four    "
                         "five    ",
                         buf),
                  0);

        buffer_destroy(b);
        window_destroy(w);
        endwin();
}

TEST(window, switch_buffers)
{
        initscr();
        window_t w = window_create(0, 0, 8, 1);

        buffer_t b1 = buffer_create();
        buffer_add_line(b1, "b1", 2);
        window_set_buffer(w, b1);
        window_refresh(w);

        char buf[12 * 4 + 1];
        window_dump(w, buf, sizeof(buf) - 1);
        ASSERT_EQ(strcmp("b1      ", buf), 0);

        buffer_t b2 = buffer_create();
        buffer_add_line(b2, "b2", 2);
        window_set_buffer(w, b2);
        window_refresh(w);

        window_dump(w, buf, sizeof(buf) - 1);
        ASSERT_EQ(strcmp("b2      ", buf), 0);

        buffer_destroy(b2);
        buffer_destroy(b1);
        window_destroy(w);
        endwin();
}
