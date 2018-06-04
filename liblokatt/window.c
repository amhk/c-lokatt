#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ncurses.h>

#include "lokatt/error.h"
#include "lokatt/format.h"
#include "lokatt/repo.h"
#include "lokatt/strbuf.h"
#include "lokatt/window.h"
#include "lokatt/wrappers.h"

struct window {
        size_t x, y, cols, rows;
        size_t lineno;

        buffer_t buffer;
        WINDOW *window;

        // scratch areas
        char *buf;
        struct strbuf strbuf;
};

window_t window_create(size_t x, size_t y, size_t cols, size_t rows)
{
        if (cols == 0) {
                cols = COLS;
        }
        if (rows == 0) {
                rows = LINES;
        }

        struct window *win = xalloc(sizeof(struct window));
        win->x = x;
        win->y = y;
        win->cols = cols;
        win->rows = rows;
        win->lineno = 0;
        win->buffer = NO_BUFFER;
        win->buf = xalloc(cols);
        strbuf_init(&win->strbuf, rows);

        if ((win->window = newwin(rows, cols, y, x)) == NULL) {
                die("newwin");
        }
        scrollok(stdscr, TRUE);     // allow scrolling
        idlok(win->window, TRUE);   // use HW insert/delete line instructions
        leaveok(win->window, TRUE); // no need to move the (invisible) cursor

        return (window_t)win;
}

void window_destroy(window_t w)
{
        struct window *win = (struct window *)w;
        delwin(win->window);
        xfree(win->buf);
        strbuf_destroy(&win->strbuf);
        xfree(win);
}

void window_set_buffer(window_t w, buffer_t b)
{
        struct window *win = (struct window *)w;
        win->buffer = b;
}

static void printw_text(struct window *win, size_t lineno, int y)
{
        if (buffer_text_get(win->buffer, lineno, win->buf, win->cols) != 0) {
                die("buffer_text_get");
        }
        mvwprintw(win->window, y, 0, "%s", win->buf);
}

static void printw_logcat(struct window *win, size_t lineno, int y)
{
        const struct logcat_entry *entry =
            buffer_logcat_peek(win->buffer, lineno);
        if (!entry) {
                // TODO: handle in a better way (entry can be
                // NULL if the repo has reclaimed memory)
                die("buffer_logcat_peek");
        }
        static const char *fmt =
            "%(4 pid)  %(4 tid)  %(-20 rtrunc tag)  %(text)";
        strbuf_reset(&win->strbuf);
        format_expand_logcat_entry(&win->strbuf, fmt, entry);
        snprintf(win->buf, win->cols, "%s", win->strbuf.buf);
        mvwprintw(win->window, y, 0, win->buf);
}

void window_refresh(window_t w)
{
        struct window *win = (struct window *)w;

        werase(win->window);

        if (win->buffer != NO_BUFFER) {
                enum buffer_type_t type = buffer_type(win->buffer);
                size_t lineno = buffer_size(win->buffer);
                int y = win->rows - 1;
                while (lineno > 0 && y >= 0) {
                        switch (type) {
                        case BUFFER_TYPE_TEXT:
                                printw_text(win, lineno, y);
                                break;
                        case BUFFER_TYPE_LOGCAT:
                                printw_logcat(win, lineno, y);
                                break;
                        default:
                                die("unexpected buffer type %d", type);
                        }
                        lineno--;
                        y--;
                }
        }

        wnoutrefresh(win->window);
}

void window_dump(window_t w, char *out, size_t n)
{
        struct window *win = (struct window *)w;
        int offset = 0;

        for (size_t y = 0; y < win->rows && n - offset > 0; y++) {
                int retval =
                    mvwinnstr(win->window, y, 0, out + offset, n - offset);
                if (retval == ERR) {
                        die("mvwinnstr");
                }
                offset += retval;
        }
}
