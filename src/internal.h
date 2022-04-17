#ifndef REAP_INTERNAL_H
#define REAP_INTERNAL_H

#include <sys/types.h>

#include <reap/definitions.h>

int
translateErrno(void);

int
betterReadlink(const char *pathname, char *buf, size_t bufsiz);

#endif  // REAP_INTERNAL_H
