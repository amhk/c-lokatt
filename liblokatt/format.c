#include <string.h>

#include "lokatt/format.h"

struct userdata_ {
        const struct logcat_entry *entry;
};

struct context {
        enum { KEY_NONE,
               KEY_PID,
               KEY_TID,
               KEY_SEC,
               KEY_NSEC,
               KEY_LEVEL,
               KEY_TAG,
               KEY_TEXT,
        } keyword;
};

static void update_context(struct context *ctx, const char *keyword, size_t len)
{
        if (!strncmp("pid", keyword, len)) {
                ctx->keyword = KEY_PID;
        } else if (!strncmp("tid", keyword, len)) {
                ctx->keyword = KEY_TID;
        } else if (!strncmp("sec", keyword, len)) {
                ctx->keyword = KEY_SEC;
        } else if (!strncmp("nsec", keyword, len)) {
                ctx->keyword = KEY_NSEC;
        } else if (!strncmp("level", keyword, len)) {
                ctx->keyword = KEY_LEVEL;
        } else if (!strncmp("tag", keyword, len)) {
                ctx->keyword = KEY_TAG;
        } else if (!strncmp("text", keyword, len)) {
                ctx->keyword = KEY_TEXT;
        }
}

static void expand_context(struct context *ctx,
                           const struct logcat_entry *entry, struct strbuf *out)
{
        switch (ctx->keyword) {
        case KEY_PID:
                strbuf_addf(out, "%d", entry->pid);
                break;
        case KEY_TID:
                strbuf_addf(out, "%d", entry->tid);
                break;
        case KEY_SEC:
                strbuf_addf(out, "%d", entry->sec);
                break;
        case KEY_NSEC:
                strbuf_addf(out, "%d", entry->nsec);
                break;
        case KEY_LEVEL:
                switch (entry->level) {
                case LOGCAT_VERBOSE:
                        strbuf_addch(out, 'V');
                        break;
                case LOGCAT_DEBUG:
                        strbuf_addch(out, 'D');
                        break;
                case LOGCAT_INFO:
                        strbuf_addch(out, 'I');
                        break;
                case LOGCAT_WARNING:
                        strbuf_addch(out, 'W');
                        break;
                case LOGCAT_ERROR:
                        strbuf_addch(out, 'E');
                        break;
                case LOGCAT_ASSERT:
                        strbuf_addch(out, 'A');
                        break;
                }
                break;
        case KEY_TAG:
                strbuf_addf(out, "%s", entry->tag);
                break;
        case KEY_TEXT:
                strbuf_addf(out, "%s", entry->text);
                break;
        case KEY_NONE:
                break;
        }
}

static size_t parse_pattern(struct strbuf *sb, const char *pattern,
                            void *userdata)
{
        if (*pattern++ != '(') {
                return 0; // missing begin (
        }

        const struct logcat_entry *entry =
            ((struct userdata_ *)userdata)->entry;
        struct context ctx = {KEY_NONE};
        const char *begin = pattern;
        const char *end;

        for (;;) {
                end = strpbrk(begin, " )");
                if (!end) {
                        return 0; // missing end )
                }

                if (end - begin > 0) {
                        update_context(&ctx, begin, end - begin);
                }

                if (*end == ')') {
                        break;
                }

                begin = end + 1;
        }

        expand_context(&ctx, entry, sb);
        return end - pattern + 2; // '(' + words + ')'
}

int format_expand_logcat_entry(struct strbuf *sb, const char *fmt,
                               const struct logcat_entry *entry)
{
        struct userdata_ ud = {entry};
        return strbuf_expand(sb, fmt, parse_pattern, &ud) > 0;
}
