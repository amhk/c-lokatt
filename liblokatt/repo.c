#include "lokatt/repo.h"
#include "lokatt/adb.h"
#include "lokatt/arena.h"
#include "lokatt/wrappers.h"

struct repo {
        struct arena indices;
        struct arena entries;
};

repo_t repo_create(void)
{
        // the average logcat entry payload is about 250 bytes

        struct repo *repo = xalloc(sizeof(*repo));
        arena_init(&repo->indices, 128 * sizeof(size_t));
        arena_init(&repo->entries, 128 * 256);
        return (repo_t)repo;
}

void repo_destroy(repo_t r)
{
        struct repo *repo = (struct repo *)r;
        arena_destroy(&repo->indices);
        arena_destroy(&repo->entries);
        xfree(repo);
}

const struct logcat_entry *repo_peek(repo_t r, size_t gid)
{
        struct repo *repo = (struct repo *)r;
        if (gid == 0 || gid > repo->indices.used_size / sizeof(size_t)) {
                return NULL;
        }

        const size_t *indices = (const size_t *)repo->indices.memory;
        const size_t offset = indices[gid - 1];
        struct logcat_entry *p = repo->entries.memory + offset;

        // logcat_entry::index is an enum: do the safe thing and use an
        // intermediate variable
        char level;
        decode_payload(p->payload, level, p->tag, p->text);
        p->level = level;

        return p;
}

size_t repo_add(repo_t r, const struct logger_entry *src)
{
        struct repo *repo = (struct repo *)r;

        const size_t offset = repo->entries.used_size;
        arena_append(&repo->indices, &offset, sizeof(size_t));

        const size_t new_size = repo->entries.used_size +
                                sizeof(struct logcat_entry) + src->payload_size;
        arena_grow(&repo->entries, new_size);
        repo->entries.used_size = new_size;

        struct logcat_entry *dest = repo->entries.memory + offset;
        dest->pid = src->pid;
        dest->tid = src->tid;
        dest->sec = src->sec;
        dest->nsec = src->nsec;
        memcpy(dest->payload, src->payload, src->payload_size);

        // no need to write the tag and text pointers here: any arena_grow
        // operation can move the entire memory area: instead, fill out tag and
        // text in repo_peek

        return (repo->indices.used_size / sizeof(size_t));
}
