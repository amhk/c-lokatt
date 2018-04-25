#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "lokatt/error.h"
#include "lokatt/strbuf.h"
#include "lokatt/wrappers.h"

/*
 * Target for newly initialized strbuf buffers. Note: global variables will be
 * initialized to 0, so this is effectively the empty string.
 */
char strbuf_default_buffer[1];

static void set_str_size(struct strbuf *sb, size_t new_size)
{
        if (new_size > (sb->alloc_size ? sb->alloc_size - 1 : 0)) {
                die("cannot set size to %zd: only %zd allocated", new_size,
                    sb->alloc_size);
        }
        sb->str_size = new_size;
        if (sb->buf != strbuf_default_buffer) {
                sb->buf[new_size] = '\0';
        }
}

void strbuf_init(struct strbuf *sb, size_t hint)
{
        sb->str_size = 0;
        sb->alloc_size = 0;
        sb->buf = strbuf_default_buffer;
        if (hint) {
                strbuf_grow(sb, hint);
        }
}

void strbuf_destroy(struct strbuf *sb)
{
        if (sb->alloc_size) {
                xfree(sb->buf);
        }
}

void strbuf_grow(struct strbuf *sb, size_t extra)
{
        if (sb->alloc_size) {
                const size_t new_alloc_size = sb->alloc_size + extra + 1;
                sb->buf = xrealloc(sb->buf, new_alloc_size);
                sb->alloc_size = new_alloc_size;
        } else {
                const size_t new_alloc_size = extra + 1;
                sb->buf = xalloc(new_alloc_size);
                sb->alloc_size = new_alloc_size;
                set_str_size(sb, 0);
        }
}

void strbuf_shrink(struct strbuf *sb, size_t size)
{
        if (!sb->alloc_size) {
                strbuf_grow(sb, 0);
        }
        if (size > sb->alloc_size) {
                die("cannot shrink from %zd to %zd", sb->str_size, size);
        }
        set_str_size(sb, size);
}

void strbuf_insert(struct strbuf *sb, size_t pos, const char *data, size_t size)
{
        strbuf_grow(sb, size);
        memmove(sb->buf + pos + size, sb->buf + pos, sb->str_size - pos);
        memcpy(sb->buf + pos, data, size);
        set_str_size(sb, sb->str_size + size);
}

void strbuf_add(struct strbuf *sb, const char *data, size_t size)
{
        strbuf_grow(sb, size);
        memcpy(sb->buf + sb->str_size, data, size);
        set_str_size(sb, sb->str_size + size);
}

void strbuf_addstr(struct strbuf *sb, const char *str)
{
        strbuf_add(sb, str, strlen(str));
}

void strbuf_addch(struct strbuf *sb, const char ch)
{
        strbuf_add(sb, &ch, 1);
}

void strbuf_vaddf(struct strbuf *sb, const char *fmt, va_list ap)
{
        size_t available =
            sb->alloc_size > 0 ? sb->alloc_size - sb->str_size - 1 : 0;

        va_list ap_cp;
        va_copy(ap_cp, ap);
        size_t required = vsnprintf(sb->buf + sb->str_size,
                                    sb->alloc_size - sb->str_size, fmt, ap_cp);
        va_end(ap_cp);

        if (required > available) {
                strbuf_grow(sb, required);
                required = vsnprintf(sb->buf + sb->str_size,
                                     sb->alloc_size - sb->str_size, fmt, ap);
        }
        set_str_size(sb, sb->str_size + required);
}

void strbuf_addf(struct strbuf *sb, const char *fmt, ...)
{
        va_list ap;

        va_start(ap, fmt);
        strbuf_vaddf(sb, fmt, ap);
        va_end(ap);
}
