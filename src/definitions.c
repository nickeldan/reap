#include <string.h>

#include <reap/definitions.h>

const char *
reapErrorString(int value)
{
    if (value < 0) {
        return strerror(-1 * value);
    }

    switch (value) {
    case REAP_RET_OK: return "No error";
    case REAP_RET_BAD_USAGE: return "Invalid API usage";
    case REAP_RET_NOT_FOUND: return "Could not find a resource";
    case REAP_RET_FILE_READ: return "Failed to read from a file";
    case REAP_RET_OTHER: return "An unanticpated error occurred";
    default: return "Invalid return value";
    }
}
