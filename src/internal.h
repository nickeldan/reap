#ifndef REAP_INTERNAL_H
#define REAP_INTERNAL_H

#include <errno.h>
#include <string.h>
#include <sys/types.h>

#include <reap/definitions.h>

#define HIDDEN_SYMBOL __attribute__((visibility("hidden")))

#ifndef NO_OP
#define NO_OP while (0)
#endif

#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))
#endif

#ifdef REAP_USE_ERROR_BUFFER

void HIDDEN_SYMBOL
emitError(const char *format, ...);
#define EMIT_ERROR(...) emitError(__VA_ARGS__)

#else

#define EMIT_ERROR(...) NO_OP

#endif

int HIDDEN_SYMBOL
translateErrno(int errno_value);

int HIDDEN_SYMBOL
betterReadlink(const char *pathname, char *buf, size_t bufsiz);

#endif  // REAP_INTERNAL_H
