#ifndef LOKATT_ADB_H
#define LOKATT_ADB_H
#include <stdint.h>
#include <string.h>

/**
 * Binary adb logcat stream parser.
 */

/**
 * A version of the logger_entry structures defined in Android's
 * system/core/include/log/log_read.h.
 */
struct logger_entry {
        uint16_t payload_size; // size of the payload
        uint16_t header_size;  // size of this struct, excluding payload
        int32_t pid;           // generating process's pid
        int32_t tid;           // generating process's tid
        int32_t sec;           // seconds since Epoch
        int32_t nsec;          // nanoseconds
        char payload[0];       // the entry's payload: <level><tag>\0<text>\0
};

#define decode_payload(payload_ptr_, level_, tag_ptr_, text_ptr_)              \
        do {                                                                   \
                (level_) = (char)(((const char *)(payload_ptr_))[0]);          \
                (tag_ptr_) =                                                   \
                    (const char *)(((const char *)(payload_ptr_)) + 1);        \
                (text_ptr_) = (const char *)(strchr((tag_ptr_), '\0') + 1);    \
                                                                               \
                /* also strip trailing newlines from text */                   \
                char *p_ = strchr(text_ptr_, '\0') - 1;                        \
                while (*p_ == '\n') {                                          \
                        *p_-- = '\0';                                          \
                }                                                              \
        } while (0)

#define LOGGER_ENTRY_MAX_PAYLOAD 4068

#define LOGGER_ENTRY_MAX_LEN (5 * 1024)

/**
 * Read the next logger_entry from an open file descriptor. Will block until
 * data becomes available; will return 0 on success, -1 on failure.
 */
int adb_next_entry(int fd, struct logger_entry *out);

#endif
