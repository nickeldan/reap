#include <reap/definitions.h>

const char *
reapErrorString(int value)
{
    switch (value) {
    case REAP_RET_OK: return "No error";
    case REAP_RET_BAD_USAGE: return "Invalid API usage";
    case REAP_RET_OUT_OF_MEMORY: return "Failed to allocate memory";
    case REAP_RET_NO_PERMISSION: return "No permission";
    case REAP_RET_NOT_FOUND: return "Could not find a resource";
    case REAP_RET_TOO_MANY_LINKS: return "Too many symbolic links to resolve";
    case REAP_RET_FILE_READ: return "Failed to read from a file";
    case REAP_RET_OTHER: return "An unanticpated error occurred";
    default: return "Invalid return value";
    }
}
