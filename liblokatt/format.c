#include <string.h>

#include "lokatt/format.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

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
        int padding;
        enum { TRUNC_NONE,
               TRUNC_LEFT,
               TRUNC_MIDDLE,
               TRUNC_RIGHT,
        } truncate;
};

static void update_context(struct context *ctx, const struct strbuf *word)
{
        char *end;
        int padding = strtol(word->buf, &end, 10);
        if (*end == '\0') {
                ctx->padding = padding;
                return;
        }

        if (!strncmp("pid", word->buf, word->str_size)) {
                ctx->keyword = KEY_PID;
        } else if (!strncmp("tid", word->buf, word->str_size)) {
                ctx->keyword = KEY_TID;
        } else if (!strncmp("sec", word->buf, word->str_size)) {
                ctx->keyword = KEY_SEC;
        } else if (!strncmp("nsec", word->buf, word->str_size)) {
                ctx->keyword = KEY_NSEC;
        } else if (!strncmp("level", word->buf, word->str_size)) {
                ctx->keyword = KEY_LEVEL;
        } else if (!strncmp("tag", word->buf, word->str_size)) {
                ctx->keyword = KEY_TAG;
        } else if (!strncmp("text", word->buf, word->str_size)) {
                ctx->keyword = KEY_TEXT;
        } else if (!strncmp("ltrunc", word->buf, word->str_size)) {
                ctx->truncate = TRUNC_LEFT;
        } else if (!strncmp("mtrunc", word->buf, word->str_size)) {
                ctx->truncate = TRUNC_MIDDLE;
        } else if (!strncmp("rtrunc", word->buf, word->str_size)) {
                ctx->truncate = TRUNC_RIGHT;
        }
}

static void expand_context(struct context *ctx,
                           const struct logcat_entry *entry, struct strbuf *out)
{
        struct strbuf sb = STRBUF_INIT;

        // expand word
        switch (ctx->keyword) {
        case KEY_PID:
                strbuf_addf(&sb, "%d", entry->pid);
                break;
        case KEY_TID:
                strbuf_addf(&sb, "%d", entry->tid);
                break;
        case KEY_SEC:
                strbuf_addf(&sb, "%d", entry->sec);
                break;
        case KEY_NSEC:
                strbuf_addf(&sb, "%d", entry->nsec);
                break;
        case KEY_LEVEL:
                switch (entry->level) {
                case LOGCAT_VERBOSE:
                        strbuf_addch(&sb, 'V');
                        break;
                case LOGCAT_DEBUG:
                        strbuf_addch(&sb, 'D');
                        break;
                case LOGCAT_INFO:
                        strbuf_addch(&sb, 'I');
                        break;
                case LOGCAT_WARNING:
                        strbuf_addch(&sb, 'W');
                        break;
                case LOGCAT_ERROR:
                        strbuf_addch(&sb, 'E');
                        break;
                case LOGCAT_ASSERT:
                        strbuf_addch(&sb, 'A');
                        break;
                }
                break;
        case KEY_TAG:
                strbuf_addf(&sb, "%s", entry->tag);
                break;
        case KEY_TEXT:
                strbuf_addf(&sb, "%s", entry->text);
                break;
        case KEY_NONE:
                break;
        }

        // add padding
        static char SPACE[] = "        ";
        if (ctx->padding < 0) {
                int spaces = -ctx->padding - sb.str_size;
                while (spaces > 0) {
                        int step = MIN(spaces, (int)sizeof(SPACE) - 1);
                        strbuf_add(&sb, SPACE, step);
                        spaces -= step;
                }
        } else if (ctx->padding > 0) {
                int spaces = ctx->padding - sb.str_size;
                while (spaces > 0) {
                        int step = MIN(spaces, (int)sizeof(SPACE) - 1);
                        strbuf_insert(&sb, 0, SPACE, step);
                        spaces -= step;
                }
        }

        // truncate
        const size_t width = abs(ctx->padding);
        if (width > 2 && width < sb.str_size) {
                switch (ctx->truncate) {
                case TRUNC_NONE:
                        break;
                case TRUNC_LEFT:
                        sb.buf[0] = '.';
                        sb.buf[1] = '.';
                        memmove(sb.buf + 2, sb.buf + sb.str_size - width + 2,
                                width - 2);
                        strbuf_shrink(&sb, width);
                        break;
                case TRUNC_MIDDLE: {
                        const size_t r = width / 2 - 1;
                        const size_t l = width - r - 1;
                        sb.buf[l - 1] = '.';
                        sb.buf[l] = '.';
                        memmove(sb.buf + l + 1, sb.buf + sb.str_size - r, r);
                        strbuf_shrink(&sb, width);
                        break;
                } break;
                case TRUNC_RIGHT:
                        strbuf_shrink(&sb, width - 2);
                        strbuf_addstr(&sb, "..");
                        break;
                }
        }

        // write result
        strbuf_add(out, sb.buf, sb.str_size);
        strbuf_destroy(&sb);
}

static size_t parse_pattern(struct strbuf *sb, const char *pattern,
                            void *userdata)
{
        if (*pattern++ != '(') {
                return 0; // missing begin (
        }

        const struct logcat_entry *entry =
            ((struct userdata_ *)userdata)->entry;
        struct context ctx = {KEY_NONE, 0, TRUNC_NONE};
        const char *begin = pattern;
        const char *end;
        struct strbuf word = STRBUF_INIT;

        for (;;) {
                end = strpbrk(begin, " )");
                if (!end) {
                        return 0; // missing end )
                }

                if (end - begin > 0) {
                        strbuf_reset(&word);
                        strbuf_add(&word, begin, end - begin);
                        update_context(&ctx, &word);
                }

                if (*end == ')') {
                        break;
                }

                begin = end + 1;
        }

        expand_context(&ctx, entry, sb);
        strbuf_destroy(&word);
        return end - pattern + 2; // '(' + words + ')'
}

int format_expand_logcat_entry(struct strbuf *sb, const char *fmt,
                               const struct logcat_entry *entry)
{
        struct userdata_ ud = {entry};
        return strbuf_expand(sb, fmt, parse_pattern, &ud) > 0;
}
