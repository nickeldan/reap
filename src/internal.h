#ifndef REAP_INTERNAL_H
#define REAP_INTERNAL_H

#include <errno.h>
#include <string.h>
#include <sys/types.h>

#include <reap/definitions.h>

#ifndef NO_OP
#define NO_OP while (0)
#endif

#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))
#endif

#ifndef REAP_NO_ERROR_BUFFER

void
emitError(const char *format, ...)
#ifdef __GNUC__
    __attribute__((format(printf, 1, 2))) __attribute__((visibility("hidden")))
#endif
    ;
#define EMIT_ERROR(...) emitError(__VA_ARGS__)

#else

#define EMIT_ERROR(...) NO_OP

#endif

int
betterReadlink(const char *pathname, char *buf, size_t bufsiz)
#ifdef __GNUC__
    __attribute__((visibility("hidden")))
#endif
    ;

#endif  // REAP_INTERNAL_H
