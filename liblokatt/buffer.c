#include <stdlib.h>
#include <string.h>

#include "lokatt/arena.h"
#include "lokatt/buffer.h"
#include "lokatt/error.h"
#include "lokatt/wrappers.h"

struct buffer {
        struct arena indices;
        struct arena entries;
};

static void buffer_init(struct buffer *buffer)
{
        arena_init(&buffer->indices, 128 * sizeof(size_t));
        arena_init(&buffer->entries, 128 * 256);
}

buffer_t buffer_create(void)
{
        struct buffer *buffer = xalloc(sizeof(struct buffer));
        buffer_init(buffer);
        return (buffer_t)buffer;
}

void buffer_destroy(buffer_t b)
{
        struct buffer *buffer = (struct buffer *)b;
        arena_destroy(&buffer->indices);
        arena_destroy(&buffer->entries);
        xfree(buffer);
}

void buffer_add_line(buffer_t b, const char *line, size_t len)
{
        struct buffer *buffer = (struct buffer *)b;

        // update entries memory
        const size_t entries_new_size = buffer->entries.used_size + len + 1;
        arena_grow(&buffer->entries, entries_new_size);
        char *p = buffer->entries.memory + buffer->entries.used_size;
        memcpy(p, line, len);
        *(p + len) = 0;

        // update indices memory
        const size_t indices_new_size =
            buffer->indices.used_size + sizeof(size_t);
        arena_grow(&buffer->indices, indices_new_size);
        size_t *q = (size_t *)((char *)buffer->indices.memory +
                               buffer->indices.used_size);
        *q = buffer->entries.used_size;

        // update entries and memory sizes
        buffer->entries.used_size = entries_new_size;
        buffer->indices.used_size = indices_new_size;
}

size_t buffer_size(buffer_t b)
{
        struct buffer *buffer = (struct buffer *)b;
        return buffer->indices.used_size / sizeof(size_t);
}

int buffer_get_line(buffer_t b, size_t lineno, char *out, size_t max_len)
{
        struct buffer *buffer = (struct buffer *)b;
        if (lineno == 0 ||
            lineno > buffer->indices.used_size / sizeof(size_t)) {
                return 1;
        }
        size_t *indices = (size_t *)buffer->indices.memory;
        size_t offset = indices[lineno - 1];
        const char *p = buffer->entries.memory + offset;
        strncpy(out, p, max_len);
        return 0;
}

void buffer_clear(buffer_t b)
{
        struct buffer *buffer = (struct buffer *)b;
        arena_destroy(&buffer->indices);
        arena_destroy(&buffer->entries);
        buffer_init(buffer);
}
