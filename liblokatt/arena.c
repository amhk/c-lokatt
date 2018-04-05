#include <assert.h>
#include <string.h>

#include "lokatt/arena.h"
#include "lokatt/wrappers.h"

void arena_init(struct arena *a, size_t step_size)
{
        a->memory = xalloc(step_size);
        a->used_size = 0;
        a->alloc_size = step_size;
        a->step_size = step_size;
}

void arena_destroy(struct arena *a)
{
        xfree(a->memory);
}

void arena_grow(struct arena *a, size_t new_min_size)
{
        assert(a->memory);

        if (a->alloc_size >= new_min_size) {
                return;
        }
        while (a->alloc_size < new_min_size) {
                a->alloc_size += a->step_size;
        }
        a->memory = xrealloc(a->memory, a->alloc_size);
}

void arena_append(struct arena *a, const void *data, size_t size)
{
        assert(a->memory);

        arena_grow(a, a->used_size + size);
        memcpy((char *)a->memory + a->used_size, data, size);
        a->used_size += size;
}
