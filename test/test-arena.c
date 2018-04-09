#include "lokatt/arena.h"

#include "test.h"

TEST(arena, calculate_offset_manually)
{
        struct arena a;
        arena_init(&a, 3 * sizeof(int));
        ASSERT_GE(a.alloc_size, 3 * sizeof(int));
        for (int i = 0; i < 10; i++) {
                arena_grow(&a, (i + 1) * sizeof(int));
                int *p = a.memory;
                *(p + i) = i;
                a.used_size += sizeof(int);
        }
        ASSERT_GE(a.alloc_size, 10 * sizeof(int));
        for (int i = 0; i < 10; i++) {
                const int *p = a.memory;
                ASSERT_EQ(*(p + i), i);
        }
        arena_destroy(&a);
}

TEST(arena, calculate_offset_automatically)
{
        struct arena a;
        arena_init(&a, 3 * sizeof(int));
        ASSERT_GE(a.alloc_size, 3 * sizeof(int));
        for (int i = 0; i < 10; i++) {
                arena_append(&a, &i, sizeof(int));
        }
        ASSERT_GE(a.alloc_size, 10 * sizeof(int));
        for (int i = 0; i < 10; i++) {
                const int *p = a.memory;
                ASSERT_EQ(*(p + i), i);
        }
        arena_destroy(&a);
}
