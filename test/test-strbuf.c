#include <stdarg.h>
#include <string.h>

#include "lokatt/strbuf.h"

#include "test.h"

#define ASSERT_STRBUF_EQ(sb, str)                                              \
        do {                                                                   \
                ASSERT_EQ(strcmp((sb)->buf, (str)), 0);                        \
                ASSERT_EQ((sb)->str_size, strlen(str));                        \
        } while (0)

TEST(strbuf, grow)
{
        struct strbuf sb = STRBUF_INIT;
        const size_t old_size = sb.alloc_size;

        ASSERT_EQ(strcmp(sb.buf, ""), 0);

        strbuf_grow(&sb, 10);
        ASSERT_GE(sb.alloc_size - 10, old_size);

        ASSERT_EQ(strcmp(sb.buf, ""), 0);

        strbuf_destroy(&sb);
}

static void f(struct strbuf *sb, const char *fmt, ...)
{
        va_list ap;

        va_start(ap, fmt);
        strbuf_vaddf(sb, fmt, ap);
        va_end(ap);
}

TEST(strbuf, add_pre_allocated)
{
        struct strbuf sb;
        strbuf_init(&sb, 100);

        ASSERT_EQ(strcmp(sb.buf, ""), 0);

        strbuf_add(&sb, "foo", 3);
        ASSERT_STRBUF_EQ(&sb, "foo");

        strbuf_addstr(&sb, "bar");
        ASSERT_STRBUF_EQ(&sb, "foobar");

        strbuf_addf(&sb, " %d '%6s'", 1234, "abc");
        ASSERT_STRBUF_EQ(&sb, "foobar 1234 '   abc'");

        f(&sb, " %d 0x%x", 42, 42);
        ASSERT_STRBUF_EQ(&sb, "foobar 1234 '   abc' 42 0x2a");

        strbuf_addch(&sb, '!');
        ASSERT_STRBUF_EQ(&sb, "foobar 1234 '   abc' 42 0x2a!");

        strbuf_destroy(&sb);
}

TEST(strbuf, add_alloc_automatically)
{
        struct strbuf sb = STRBUF_INIT;

        ASSERT_EQ(strcmp(sb.buf, ""), 0);

        strbuf_add(&sb, "foo", 3);
        ASSERT_STRBUF_EQ(&sb, "foo");

        strbuf_addstr(&sb, "bar");
        ASSERT_STRBUF_EQ(&sb, "foobar");

        strbuf_addf(&sb, " %d '%s'", 1234, "abc");
        ASSERT_STRBUF_EQ(&sb, "foobar 1234 'abc'");

        strbuf_destroy(&sb);
}

TEST(strbuf, shrink)
{
        struct strbuf sb = STRBUF_INIT;

        ASSERT_EQ(strcmp(sb.buf, ""), 0);

        strbuf_addstr(&sb, "foo");
        ASSERT_STRBUF_EQ(&sb, "foo");

        strbuf_shrink(&sb, 2);
        ASSERT_STRBUF_EQ(&sb, "fo");

        strbuf_shrink(&sb, 1);
        ASSERT_STRBUF_EQ(&sb, "f");

        strbuf_shrink(&sb, 0);
        ASSERT_STRBUF_EQ(&sb, "");

        strbuf_addstr(&sb, "bar");
        ASSERT_STRBUF_EQ(&sb, "bar");

        strbuf_reset(&sb);
        ASSERT_STRBUF_EQ(&sb, "");

        strbuf_destroy(&sb);
}

TEST(strbuf, insert)
{
        struct strbuf sb = STRBUF_INIT;

        strbuf_insert(&sb, 0, "foo", 3);
        ASSERT_EQ(strcmp(sb.buf, "foo"), 0);

        strbuf_insert(&sb, 0, "abc-", 4);
        ASSERT_STRBUF_EQ(&sb, "abc-foo");

        strbuf_insert(&sb, 7, "-xyz", 4);
        ASSERT_STRBUF_EQ(&sb, "abc-foo-xyz");

        strbuf_insert(&sb, 7, "bar", 3);
        ASSERT_STRBUF_EQ(&sb, "abc-foobar-xyz");

        strbuf_destroy(&sb);
}

static size_t exp_inc(struct strbuf *sb, const char *pattern, void *userdata)
{
        if (*pattern == 'n') {
                int *i = (int *)userdata;
                strbuf_addf(sb, "%d", *i);
                *i += 1;
                return 1;
        }
        return 0;
}

TEST(strbuf, expand)
{
        struct strbuf sb = STRBUF_INIT;
        int i = 0;

        int status = strbuf_expand(&sb, "foo", exp_inc, &i);
        ASSERT_EQ(status, 0);
        ASSERT_STRBUF_EQ(&sb, "foo");

        i = 0;
        strbuf_reset(&sb);
        status = strbuf_expand(&sb, "%%", exp_inc, &i);
        ASSERT_EQ(status, 0);
        ASSERT_STRBUF_EQ(&sb, "%");

        i = 0;
        strbuf_reset(&sb);
        status = strbuf_expand(&sb, "%%%%", exp_inc, &i);
        ASSERT_EQ(status, 0);
        ASSERT_STRBUF_EQ(&sb, "%%");

        i = 0;
        strbuf_reset(&sb);
        status = strbuf_expand(&sb, "%n", exp_inc, &i);
        ASSERT_EQ(status, 0);
        ASSERT_STRBUF_EQ(&sb, "0");

        i = 8;
        strbuf_reset(&sb);
        status = strbuf_expand(&sb, "%n %n %n %n", exp_inc, &i);
        ASSERT_EQ(status, 0);
        ASSERT_STRBUF_EQ(&sb, "8 9 10 11");

        i = 0;
        strbuf_reset(&sb);
        status = strbuf_expand(&sb, "%x", exp_inc, &i);
        ASSERT_NE(status, 0);

        strbuf_destroy(&sb);
}
