#ifndef REAP_TESTS_COMMON_H
#define REAP_TESTS_COMMON_H

#include <reap/reap.h>

#ifdef REAP_NO_ERROR_BUFFER
#define ERROR(ret) reapErrorString(ret)
#else
#define ERROR(ret) reapGetError()
#endif

#endif  // REAP_TESTS_COMMON_H
