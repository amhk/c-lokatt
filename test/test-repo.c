#include <stdio.h>

#include "lokatt/adb.h"
#include "lokatt/repo.h"

#include "test.h"

static void logger_entry_init(struct logger_entry *entry, int32_t pid,
                              int32_t tid, int32_t sec, int32_t nsec,
                              char level, const char *tag, const char *text)
{
        entry->payload_size = 1 + strlen(tag) + 1 + strlen(text) + 1;
        entry->header_size = sizeof(struct logger_entry);
        entry->pid = pid;
        entry->tid = tid;
        entry->sec = sec;
        entry->nsec = nsec;
        char *p = entry->payload;
        *p++ = level;
        strcpy(p, tag);
        p += strlen(tag) + 1;
        strcpy(p, text);
}

TEST(repo, single_entry)
{
        char buf[LOGGER_ENTRY_MAX_LEN];
        struct logger_entry *src = (struct logger_entry *)buf;
        logger_entry_init(src, 1, 2, 3, 4, LOGCAT_INFO, "some-tag",
                          "some-text");

        repo_t repo = repo_create();
        const size_t gid = repo_add(repo, src);
        const struct logcat_entry *dest = repo_peek(repo, gid);
        ASSERT_NE(dest, NULL);
        ASSERT_EQ(dest->pid, 1);
        ASSERT_EQ(dest->tid, 2);
        ASSERT_EQ(dest->sec, 3);
        ASSERT_EQ(dest->nsec, 4);
        ASSERT_EQ(dest->level, LOGCAT_INFO);
        ASSERT_EQ(strcmp(dest->tag, "some-tag"), 0);
        ASSERT_EQ(strcmp(dest->text, "some-text"), 0);

        repo_destroy(repo);
}

TEST(repo, multiple_entries)
{
        char gids[10];
        repo_t repo = repo_create();

        for (int32_t i = 0; (size_t)i < sizeof(gids); i++) {
                char buf[LOGGER_ENTRY_MAX_LEN];
                struct logger_entry *src = (struct logger_entry *)buf;
                char tag[16];
                char text[16];

                snprintf(tag, sizeof(tag), "tag-%d", i);
                snprintf(text, sizeof(text), "text-%d", i);
                logger_entry_init(src, i, i, i, i, LOGCAT_DEBUG, tag, text);

                gids[i] = repo_add(repo, src);
        }

        for (int32_t i = 0; (size_t)i < sizeof(gids); i++) {
                const struct logcat_entry *dest = repo_peek(repo, gids[i]);
                char tag[16];
                char text[16];

                snprintf(tag, sizeof(tag), "tag-%d", i);
                snprintf(text, sizeof(text), "text-%d", i);

                ASSERT_NE(dest, NULL);
                ASSERT_EQ(dest->pid, i);
                ASSERT_EQ(dest->tid, i);
                ASSERT_EQ(dest->sec, i);
                ASSERT_EQ(dest->nsec, i);
                ASSERT_EQ(dest->level, LOGCAT_DEBUG);
                ASSERT_EQ(strcmp(dest->tag, tag), 0);
                ASSERT_EQ(strcmp(dest->text, text), 0);
        }

        repo_destroy(repo);
}
