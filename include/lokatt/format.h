#ifndef LOKATT_FORMAT_H
#define LOKATT_FORMAT_H
#include "lokatt/repo.h"
#include "lokatt/strbuf.h"

int format_expand_logcat_entry(struct strbuf *sb, const char *fmt,
                               const struct logcat_entry *entry);

#endif
