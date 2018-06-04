#include <string.h>

#include "lokatt/format.h"
#include "lokatt/strbuf.h"

#include "test.h"

static const struct logcat_entry entry = {
    1, 2, 3, 4, LOGCAT_DEBUG, "some-tag", "This is the text", {}};

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
        ASSERT_EQ(strcmp(sb.buf, "This is the text"), 0);

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

TEST(format, expand_truncate)
{
        struct strbuf sb = STRBUF_INIT;

        int status =
            format_expand_logcat_entry(&sb, "%(text 12 ltrunc)", &entry);
        ASSERT_EQ(status, 0);
        ASSERT_EQ(strcmp(sb.buf, "..s the text"), 0);

        strbuf_reset(&sb);
        status = format_expand_logcat_entry(&sb, "%(text 12 mtrunc)", &entry);
        ASSERT_EQ(status, 0);
        ASSERT_EQ(strcmp(sb.buf, "This .. text"), 0);

        strbuf_reset(&sb);
        status = format_expand_logcat_entry(&sb, "%(text 11 mtrunc)", &entry);
        ASSERT_EQ(status, 0);
        ASSERT_EQ(strcmp(sb.buf, "This ..text"), 0);

        strbuf_reset(&sb);
        status = format_expand_logcat_entry(&sb, "%(text 10 mtrunc)", &entry);
        ASSERT_EQ(status, 0);
        ASSERT_EQ(strcmp(sb.buf, "This..text"), 0);

        strbuf_reset(&sb);
        status = format_expand_logcat_entry(&sb, "%(text 12 rtrunc)", &entry);
        ASSERT_EQ(status, 0);
        ASSERT_EQ(strcmp(sb.buf, "This is th.."), 0);

        strbuf_destroy(&sb);
}

TEST(format, no_context_spillover_to_next_pattern)
{
        struct strbuf sb = STRBUF_INIT;

        int status = format_expand_logcat_entry(
            &sb, "pid=%(2 pid) tag=%(-6 rtrunc tag) text=%(text)", &entry);
        ASSERT_EQ(status, 0);
        ASSERT_EQ(strcmp(sb.buf, "pid= 1 tag=some.. text=This is the text"), 0);

        strbuf_reset(&sb);
        status = format_expand_logcat_entry(
            &sb, "%(2 pid)|%(-18 rtrunc tag)|%(text)", &entry);

        ASSERT_EQ(status, 0);
        ASSERT_EQ(strcmp(sb.buf, " 1|some-tag          |This is the text"), 0);

        strbuf_destroy(&sb);
}
