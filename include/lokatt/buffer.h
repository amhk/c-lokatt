#ifndef LOKATT_BUFFER_H
#define LOKATT_BUFFER_H
#include <stddef.h>
#include <stdint.h>

#include "lokatt/repo.h"

/**
 * Dynamically allocated and expanding container of text.
 *
 * In its current implementation, a buffer works with plain strings and
 * contains its data in a contiguous block of memory which is realloc'd as
 * needed. When lokatt is more complete, a buffer should work with logcat
 * entries (a simpler version of Android's logger_entry structs). These should
 * be kept in a central repository, and buffers should only track the indices
 * of these entries.
 *
 * Buffers should also support filtering.
 */

typedef uintptr_t buffer_t;

#define NO_BUFFER 0

enum buffer_type_t {
        BUFFER_TYPE_TEXT,
        BUFFER_TYPE_LOGCAT,
};

/**
 * Create a new, empty buffer. The caller becomes the owner of the buffer.
 *
 * The buffer will hold a reference to a repo, but will not own the repo. The
 * repo must outlive the buffer.
 */
buffer_t buffer_text_create();
buffer_t buffer_logcat_create(repo_t repo);

/**
 * Destroy a buffer.
 */
void buffer_destroy(buffer_t);

/**
 * The number of logcat entries added to the buffer.
 */
size_t buffer_size(buffer_t);

enum buffer_type_t buffer_type(buffer_t);

/**
 * Reset the buffer to its initial empty state.
 */
void buffer_clear(buffer_t);

/**
 * Add a line to the buffer.
 */
void buffer_text_add(buffer_t, const char *line, size_t len);

/**
 * Get a line from the buffer.
 */
int buffer_text_get(buffer_t, size_t lineno, char *out, size_t max_len);

int buffer_logcat_accept(buffer_t, size_t gid,
                         const struct logcat_entry *entry);
const struct logcat_entry *buffer_logcat_peek(buffer_t, size_t lineno);

#endif
