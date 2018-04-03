#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "lokatt/adb.h"

#include "test.h"

TEST(adb, read_from_file)
{
        static const char *filename =
            "pixel-2-android-p-developer-preview-easter-egg.bin";
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", test_data_dir(), filename);
        int fd = open(path, O_RDONLY);
        ASSERT_NE(fd, -1);

        char buf[LOGGER_ENTRY_MAX_LEN];
        for (int i = 0; i < 15; i++) {
                int status = adb_next_entry(fd, (struct logger_entry *)buf);
                ASSERT_NE(status, -1);
        }

        const struct logger_entry *entry = (const struct logger_entry *)&buf;
        char level;
        const char *tag, *text;
        decode_payload(entry->payload, level, tag, text);

        ASSERT_EQ(entry->payload_size, 58);
        ASSERT_EQ(entry->header_size, 28);
        ASSERT_EQ(entry->pid, 1179);
        ASSERT_EQ(entry->tid, 1306);
        ASSERT_EQ(entry->sec, 1522927823);
        ASSERT_EQ(entry->nsec, 165861320);
        ASSERT_EQ(level, 6);
        ASSERT_EQ(strcmp(tag, "RadioChainInfo"), 0);
        ASSERT_EQ(strcmp(text, "Found trailing data after parcel parsing."), 0);

        close(fd);
}
