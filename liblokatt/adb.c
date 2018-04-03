#include <assert.h>
#include <unistd.h>

#include "lokatt/adb.h"

static ssize_t xread(int fd, char *buf, size_t count)
{
        const size_t requested_count = count;
        buf += count;
        for (;;) {
                ssize_t r = read(fd, buf - count, count);
                if (r <= 0) {
                        return r;
                }
                count -= r;
                if (count == 0) {
                        return requested_count;
                }
        }
}

int adb_next_entry(int fd, struct logger_entry *out)
{
        ssize_t r = xread(fd, (char *)out, sizeof(struct logger_entry));
        if (r <= 0) {
                return -1;
        }

        static char dummy[8];
        size_t skip = out->header_size - sizeof(struct logger_entry);
        if (skip > 0) {
                assert(skip <= sizeof(dummy));
                r = xread(fd, dummy, sizeof(dummy));
                if (r <= 0) {
                        return -1;
                }
        }

        r = xread(fd, (char *)out->payload, out->payload_size);
        if (r <= 0) {
                return -1;
        }

        return 0;
}
