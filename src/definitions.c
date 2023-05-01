#include <stdarg.h>
#include <stdio.h>

#include <reap/reap.h>

#include "internal.h"

static _Thread_local char errorBuffer[REAP_ERROR_BUFFER_SIZE] = {0};

void
reapEmitError(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    vsnprintf(errorBuffer, sizeof(errorBuffer), format, args);
    va_end(args);
}

char *
reapGetError(void)
{
    return errorBuffer;
}
