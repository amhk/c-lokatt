#ifndef LOKATT_REPO_H
#define LOKATT_REPO_H
#include <stddef.h>
#include <stdint.h>

/**
 * A repository of logcat entries. This is the data store that backs buffers and
 * contains all logcat entries read via adb.
 *
 * A logcat entry is similar to a logger entry, but is more lokatt friendly in
 * terms of memory consumtion and fields exposed.
 */

struct logger_entry;

struct logcat_entry {
        int32_t pid;
        int32_t tid;
        int32_t sec;
        int32_t nsec;
        enum { LOGCAT_VERBOSE = 2,
               LOGCAT_DEBUG,
               LOGCAT_INFO,
               LOGCAT_WARNING,
               LOGCAT_ERROR,
               LOGCAT_ASSERT,
        } level;
        const char *tag;
        const char *text;
        char payload[0];
};

typedef uintptr_t repo_t;

repo_t repo_create(void);

void repo_destroy(repo_t);

/**
 * Get a pointer to the logcat entry identified by gid. The caller must not
 * store the pointer as the entry may be reallocated or freed.
 *
 * Return NULL if the repo does not recognize the gid.
 */
const struct logcat_entry *repo_peek(repo_t, size_t gid);

/**
 * Insert a new logcat entry into the repo. Return the gid that was assigned
 * the entry.
 */
size_t repo_add(repo_t, const struct logger_entry *entry);

#endif
