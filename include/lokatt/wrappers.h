#ifndef LOKATT_WRAPPER_H
#define LOKATT_WRAPPER_H
#include <stdlib.h>

#include "lokatt/error.h"

static inline void *xalloc(size_t size)
{
        void *p = calloc(1, size);
        if (!p) {
                die("xmalloc");
        }
        return p;
}

static inline void xfree(void *p)
{
        free(p);
}

static inline void *xrealloc(void *ptr, size_t size)
{
        ptr = realloc(ptr, size);
        if (!ptr) {
                die("xrealloc");
        }
        return ptr;
}

#endif
