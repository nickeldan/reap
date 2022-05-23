#ifndef REAP_TESTS_COMMON_H
#define REAP_TESTS_COMMON_H

#include <reap/definitions.h>

#ifdef REAP_USE_ERROR_BUFFER
#define ERROR(ret) reapGetError()
#else
#define ERROR(ret) reapErrorString(ret)
#endif

#endif  // REAP_TESTS_COMMON_H
