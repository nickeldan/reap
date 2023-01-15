#ifndef REAP_INTERNAL_H
#define REAP_INTERNAL_H

#include <errno.h>
#include <string.h>
#include <sys/types.h>

#include <reap/definitions.h>

#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))
#endif

void
emitError(const char *format, ...)
#ifdef __GNUC__
    __attribute__((format(printf, 1, 2))) __attribute__((visibility("hidden")))
#endif
    ;
#define EMIT_ERROR(...) emitError(__VA_ARGS__)

int
betterReadlink(const char *pathname, char *buf, size_t bufsiz)
#ifdef __GNUC__
    __attribute__((visibility("hidden")))
#endif
    ;

#endif  // REAP_INTERNAL_H
