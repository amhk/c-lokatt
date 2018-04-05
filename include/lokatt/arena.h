#ifndef LOKATT_ARENA_H
#define LOKATT_ARENA_H
#include <unistd.h>

/**
 * A dynamically allocated block of memory that can be reallocated to a larger
 * size.
 *
 * The caller can modify the memory directly via memory and used_size or call
 * the arena_append convenience function. Note that after a call to arena_grow,
 * the memory pointer may have changed.
 */

struct arena {
        size_t alloc_size;
        size_t step_size;
        size_t used_size;
        void *memory;
};

/**
 * Create a new arena. Memory will be allocated in step_size byte increments;
 * the initial memory allocation size is step_size bytes.
 */
void arena_init(struct arena *a, size_t step_size);

void arena_destroy(struct arena *a);

void arena_grow(struct arena *a, size_t new_min_size);

/**
 * Convenience method to write data the end of the used part of the memory
 * block. If needed, the arena will grow to accommodate the new data.
 */
void arena_append(struct arena *a, const void *data, size_t size);

#endif
