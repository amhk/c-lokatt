#ifndef LOKATT_BUFFER_H
#define LOKATT_BUFFER_H
#include <stddef.h>
#include <stdint.h>

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

/**
 * Create a new, empty buffer. The caller becomes the owner of the buffer.
 */
buffer_t buffer_create(void);

/**
 * Destroy a buffer.
 */
void buffer_destroy(buffer_t);

/**
 * The number of lines added to the buffer.
 */
size_t buffer_size(buffer_t);

/**
 * Add a line to the buffer.
 */
void buffer_add_line(buffer_t, const char *line, size_t len);

/**
 * Get a line from the buffer.
 */
int buffer_get_line(buffer_t, size_t lineno, char *out, size_t max_len);

/**
 * Reset the buffer to its initial empty state.
 */
void buffer_clear(buffer_t);

#endif
