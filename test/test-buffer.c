#include <stdio.h>
#include <string.h>

#include "lokatt/buffer.h"
#include "lokatt/repo.h"

#include "test.h"

TEST(buffer, text_basic_insert)
{
        buffer_t b = buffer_text_create();
        ASSERT_EQ(buffer_size(b), 0);

        buffer_text_add(b, "foobar", 6);
        ASSERT_EQ(buffer_size(b), 1);

        buffer_text_add(b, "test", 4);
        ASSERT_EQ(buffer_size(b), 2);

        buffer_destroy(b);
}

TEST(buffer, text_basic_get)
{
        buffer_t b = buffer_text_create();
        buffer_text_add(b, "foo", 3);
        buffer_text_add(b, "bar", 3);

        char buf[16];
        int ok = buffer_text_get(b, 0, buf, sizeof(buf));
        ASSERT_NE(ok, 0);

        ok = buffer_text_get(b, 1, buf, sizeof(buf));
        ASSERT_EQ(ok, 0);
        ASSERT_EQ(strcmp("foo", buf), 0);

        ok = buffer_text_get(b, 2, buf, sizeof(buf));
        ASSERT_EQ(ok, 0);
        ASSERT_EQ(strcmp("bar", buf), 0);

        ok = buffer_text_get(b, 3, buf, sizeof(buf));
        ASSERT_NE(ok, 0);

        buffer_destroy(b);
}

TEST(buffer, text_stress_insert)
{
        buffer_t b = buffer_text_create();

        for (size_t i = 0; i < 2048; i++) {
                ASSERT_EQ(i, buffer_size(b));

                char buf[128];
                snprintf(buf, sizeof(buf), "line %zu", i);
                buffer_text_add(b, buf, strlen(buf));
        }

        buffer_clear(b);

        for (size_t i = 0; i < 2048; i++) {
                ASSERT_EQ(i, buffer_size(b));

                char buf[128];
                snprintf(buf, sizeof(buf), "line %zu", i);
                buffer_text_add(b, buf, strlen(buf));
        }

        buffer_destroy(b);
}

TEST(buffer, logcat_basic_insert)
{
        repo_t r = repo_create();
        buffer_t b = buffer_logcat_create(r);

        // FIXME: continue here
        // - assert size == 0
        // - create logger entry (extract logger_entry_init from test-repo.c
        // (and add tests to test-adb.c))
        // - add logger entry to repo
        // - buffer_logcat_accept
        // - assert size == 1
        // - peek logcat entry

        buffer_destroy(b);
        repo_destroy(r);
}
