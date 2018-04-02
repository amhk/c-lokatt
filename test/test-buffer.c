#include <stdio.h>
#include <string.h>

#include "lokatt/buffer.h"

#include "test.h"

TEST(buffer, basic_insert)
{
        buffer_t b = buffer_create();
        ASSERT_EQ(buffer_size(b), 0);

        buffer_add_line(b, "foobar", 6);
        ASSERT_EQ(buffer_size(b), 1);

        buffer_add_line(b, "test", 4);
        ASSERT_EQ(buffer_size(b), 2);

        buffer_destroy(b);
}

TEST(buffer, basic_get)
{
        buffer_t b = buffer_create();
        buffer_add_line(b, "foo", 3);
        buffer_add_line(b, "bar", 3);

        char buf[16];
        int ok = buffer_get_line(b, 0, buf, sizeof(buf));
        ASSERT_NE(ok, 0);

        ok = buffer_get_line(b, 1, buf, sizeof(buf));
        ASSERT_EQ(ok, 0);
        ASSERT_EQ(strcmp("foo", buf), 0);

        ok = buffer_get_line(b, 2, buf, sizeof(buf));
        ASSERT_EQ(ok, 0);
        ASSERT_EQ(strcmp("bar", buf), 0);

        ok = buffer_get_line(b, 3, buf, sizeof(buf));
        ASSERT_NE(ok, 0);

        buffer_destroy(b);
}

TEST(buffer, stress_insert)
{
        buffer_t b = buffer_create();

        for (size_t i = 0; i < 2048; i++) {
                ASSERT_EQ(i, buffer_size(b));

                char buf[128];
                snprintf(buf, sizeof(buf), "line %zu", i);
                buffer_add_line(b, buf, strlen(buf));
        }

        buffer_clear(b);

        for (size_t i = 0; i < 2048; i++) {
                ASSERT_EQ(i, buffer_size(b));

                char buf[128];
                snprintf(buf, sizeof(buf), "line %zu", i);
                buffer_add_line(b, buf, strlen(buf));
        }

        buffer_destroy(b);
}
