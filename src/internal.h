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
#define REAP_HIDDEN      __attribute__((visibility("hidden")))
#else
#define REAP_FORMAT(pos)
#define REAP_HIDDEN
#endif

void
emitError(const char *format, ...) REAP_FORMAT(1) REAP_HIDDEN;

int
betterReadlink(const char *pathname, char *buf, size_t bufsiz) REAP_HIDDEN;

#endif  // REAP_INTERNAL_H
