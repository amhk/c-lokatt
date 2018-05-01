#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lokatt/error.h"

void __die(const char *file, unsigned int line, const char *func,
           const char *fmt, ...)
{
        va_list ap;
        va_start(ap, fmt);
        error_vprint(file, line, func, fmt, ap);
        va_end(ap);

        abort();
}

void error_vprint(const char *file, unsigned int line, const char *func,
                  const char *fmt, va_list ap)
{
        fprintf(stderr, "%s:%d:%s: ", file, line, func);
        vfprintf(stderr, fmt, ap);
        if (errno) {
                fprintf(stderr, ": %d %s", errno, strerror(errno));
        }
        fprintf(stderr, "\n");
}
