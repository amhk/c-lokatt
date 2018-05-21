#include <string.h>

#include "lokatt/format.h"
#include "lokatt/strbuf.h"

#include "test.h"

static const struct logcat_entry entry = {
    1, 2, 3, 4, LOGCAT_DEBUG, "some-tag", "some-text", {}};

TEST(format, basic_expand_logcat_entry)
{
        struct strbuf sb = STRBUF_INIT;

        int status = format_expand_logcat_entry(&sb, "", &entry);
        ASSERT_EQ(status, 0);
        ASSERT_EQ(strcmp(sb.buf, ""), 0);

        strbuf_reset(&sb);
        status = format_expand_logcat_entry(&sb, "foo", &entry);
        ASSERT_EQ(status, 0);
        ASSERT_EQ(strcmp(sb.buf, "foo"), 0);

        strbuf_reset(&sb);
        status = format_expand_logcat_entry(&sb, "%(tag)", &entry);
        ASSERT_EQ(status, 0);
        ASSERT_EQ(strcmp(sb.buf, "some-tag"), 0);

        strbuf_reset(&sb);
        status = format_expand_logcat_entry(&sb, "%(pid) %(tid)", &entry);
        ASSERT_EQ(status, 0);
        ASSERT_EQ(strcmp(sb.buf, "1 2"), 0);

        strbuf_destroy(&sb);
}

TEST(format, expand_each_entry_field)
{
        struct strbuf sb = STRBUF_INIT;

        int status = format_expand_logcat_entry(&sb, "%(pid)", &entry);
        ASSERT_EQ(status, 0);
        ASSERT_EQ(strcmp(sb.buf, "1"), 0);

        strbuf_reset(&sb);
        status = format_expand_logcat_entry(&sb, "%(tid)", &entry);
        ASSERT_EQ(status, 0);
        ASSERT_EQ(strcmp(sb.buf, "2"), 0);

        strbuf_reset(&sb);
        status = format_expand_logcat_entry(&sb, "%(sec)", &entry);
        ASSERT_EQ(status, 0);
        ASSERT_EQ(strcmp(sb.buf, "3"), 0);

        strbuf_reset(&sb);
        status = format_expand_logcat_entry(&sb, "%(nsec)", &entry);
        ASSERT_EQ(status, 0);
        ASSERT_EQ(strcmp(sb.buf, "4"), 0);

        strbuf_reset(&sb);
        status = format_expand_logcat_entry(&sb, "%(level)", &entry);
        ASSERT_EQ(status, 0);
        ASSERT_EQ(strcmp(sb.buf, "D"), 0);

        strbuf_reset(&sb);
        status = format_expand_logcat_entry(&sb, "%(tag)", &entry);
        ASSERT_EQ(status, 0);
        ASSERT_EQ(strcmp(sb.buf, "some-tag"), 0);

        strbuf_reset(&sb);
        status = format_expand_logcat_entry(&sb, "%(text)", &entry);
        ASSERT_EQ(status, 0);
        ASSERT_EQ(strcmp(sb.buf, "some-text"), 0);

        strbuf_destroy(&sb);
}

TEST(format, expand_padding)
{
        struct strbuf sb = STRBUF_INIT;

        int status = format_expand_logcat_entry(&sb, "%(pid 4)", &entry);
        ASSERT_EQ(status, 0);
        ASSERT_EQ(strcmp(sb.buf, "   1"), 0);

        strbuf_reset(&sb);
        status = format_expand_logcat_entry(&sb, "%(pid -4)", &entry);
        ASSERT_EQ(status, 0);
        ASSERT_EQ(strcmp(sb.buf, "1   "), 0);

        status = format_expand_logcat_entry(&sb, "%(tag -10)", &entry);
        ASSERT_EQ(status, 0);
        ASSERT_EQ(strcmp(sb.buf, "1   some-tag  "), 0);

        status = format_expand_logcat_entry(&sb, "*%(2 level)*", &entry);
        ASSERT_EQ(status, 0);
        ASSERT_EQ(strcmp(sb.buf, "1   some-tag  * D*"), 0);

        strbuf_destroy(&sb);
}
