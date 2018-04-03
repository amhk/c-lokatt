#include <stdlib.h>
#include <string.h>

#include "lokatt/arena.h"
#include "lokatt/buffer.h"
#include "lokatt/error.h"
#include "lokatt/repo.h"
#include "lokatt/wrappers.h"

struct buffer;

struct buffer_ops {
        enum buffer_type_t type;
        void (*init)(struct buffer *);
        void (*destroy)(struct buffer *);
        size_t (*size)(struct buffer *);
        void (*clear)(struct buffer *);
};

static void text_init(struct buffer *);
static void text_destroy(struct buffer *);
static size_t text_size(struct buffer *);
static void text_clear(struct buffer *);

static struct buffer_ops text_ops = {
    .type = BUFFER_TYPE_TEXT,
    .init = text_init,
    .destroy = text_destroy,
    .size = text_size,
    .clear = text_clear,
};

static void logcat_init(struct buffer *);
static void logcat_destroy(struct buffer *);
static size_t logcat_size(struct buffer *);
static void logcat_clear(struct buffer *);

static struct buffer_ops logcat_ops = {
    .type = BUFFER_TYPE_LOGCAT,
    .init = logcat_init,
    .destroy = logcat_destroy,
    .size = logcat_size,
    .clear = logcat_clear,
};

struct buffer {
        struct buffer_ops ops;
        union {
                struct {
                        struct arena indices;
                        struct arena entries;
                } text;
                struct {
                        repo_t repo;
                        struct arena gids;
                } logcat;
        };
};

// generic ops

void buffer_destroy(buffer_t b)
{
        struct buffer *buffer = (struct buffer *)b;
        buffer->ops.destroy(buffer);
}

enum buffer_type_t buffer_type(buffer_t b)
{
        struct buffer *buffer = (struct buffer *)b;
        return buffer->ops.type;
}

size_t buffer_size(buffer_t b)
{
        struct buffer *buffer = (struct buffer *)b;
        return buffer->ops.size(buffer);
}

void buffer_clear(buffer_t b)
{
        struct buffer *buffer = (struct buffer *)b;
        return buffer->ops.clear(buffer);
}

// text ops

buffer_t buffer_text_create()
{
        struct buffer *buffer = xalloc(sizeof(struct buffer));
        buffer->ops = text_ops;
        buffer->ops.init(buffer);
        return (buffer_t)buffer;
}

static void text_init(struct buffer *buffer)
{
        arena_init(&buffer->text.indices, 128 * sizeof(size_t));
        arena_init(&buffer->text.entries, 128 * 256);
}

static void text_destroy(struct buffer *buffer)
{
        arena_destroy(&buffer->text.indices);
        arena_destroy(&buffer->text.entries);
        xfree(buffer);
}

static size_t text_size(struct buffer *buffer)
{
        return buffer->text.indices.used_size / sizeof(size_t);
}

static void text_clear(struct buffer *buffer)
{
        arena_destroy(&buffer->text.indices);
        arena_destroy(&buffer->text.entries);
        buffer->ops.init(buffer);
}

void buffer_text_add(buffer_t b, const char *line, size_t len)
{

        struct buffer *buffer = (struct buffer *)b;

        // update entries memory
        const size_t entries_new_size =
            buffer->text.entries.used_size + len + 1;
        arena_grow(&buffer->text.entries, entries_new_size);
        char *p = buffer->text.entries.memory + buffer->text.entries.used_size;
        memcpy(p, line, len);
        *(p + len) = 0;

        // update indices memory
        const size_t indices_new_size =
            buffer->text.indices.used_size + sizeof(size_t);
        arena_grow(&buffer->text.indices, indices_new_size);
        size_t *q = (size_t *)((char *)buffer->text.indices.memory +
                               buffer->text.indices.used_size);
        *q = buffer->text.entries.used_size;

        // update entries and memory sizes
        buffer->text.entries.used_size = entries_new_size;
        buffer->text.indices.used_size = indices_new_size;
}

int buffer_text_get(buffer_t b, size_t lineno, char *out, size_t max_len)
{
        struct buffer *buffer = (struct buffer *)b;
        if (lineno == 0 ||
            lineno > buffer->text.indices.used_size / sizeof(size_t)) {
                return -1;
        }
        size_t *indices = (size_t *)buffer->text.indices.memory;
        size_t offset = indices[lineno - 1];
        const char *p = buffer->text.entries.memory + offset;
        strncpy(out, p, max_len);
        return 0;
}

// logcat ops

buffer_t buffer_logcat_create(repo_t repo)
{
        struct buffer *buffer = xalloc(sizeof(struct buffer));
        buffer->ops = logcat_ops;
        buffer->logcat.repo = repo;
        buffer->ops.init(buffer);
        return (buffer_t)buffer;
}

static void logcat_init(struct buffer *buffer)
{
        arena_init(&buffer->logcat.gids, 256 * sizeof(size_t));
}

void logcat_destroy(struct buffer *buffer)
{
        arena_destroy(&buffer->logcat.gids);
        xfree(buffer);
}

size_t logcat_size(struct buffer *buffer)
{
        return buffer->logcat.gids.used_size / sizeof(size_t);
}

void logcat_clear(struct buffer *buffer)
{
        arena_destroy(&buffer->logcat.gids);
        buffer->ops.init(buffer);
}

int buffer_logcat_accept(buffer_t b, size_t gid,
                         const struct logcat_entry *entry
                         __attribute__((unused)))
{
        // TODO: add filter here, return -1 if entry does not pass filter
        struct buffer *buffer = (struct buffer *)b;
        arena_append(&buffer->logcat.gids, &gid, sizeof(gid));
        return 0;
}

const struct logcat_entry *buffer_logcat_peek(buffer_t b, size_t lineno)
{
        struct buffer *buffer = (struct buffer *)b;
        if (lineno == 0 ||
            lineno > buffer->logcat.gids.used_size / sizeof(size_t)) {
                return NULL;
        }
        const size_t *gids = (const size_t *)buffer->logcat.gids.memory;
        const size_t gid = gids[lineno - 1];
        return repo_peek(buffer->logcat.repo, gid);
}
