#ifndef REAP_INTERNAL_H
#define REAP_INTERNAL_H

#include <errno.h>
#include <string.h>
#include <sys/types.h>

#include <reap/definitions.h>

#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))
#endif

#ifdef __GNUC__
#define REAP_FORMAT(pos) __attribute__((format(printf, pos, pos + 1)))
#else
#define REAP_FORMAT(pos)
#endif

void
reapEmitError(const char *format, ...) REAP_FORMAT(1);

int
reapBetterReadlink(const char *pathname, char *buf, size_t bufsiz);

#endif  // REAP_INTERNAL_H
