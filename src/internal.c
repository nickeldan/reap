#include <errno.h>

#include "internal.h"

int
translateErrno(void)
{
    switch (errno) {
    case EACCES: return REAP_RET_NO_PERMISSION;
    case ELOOP: return REAP_RET_TOO_MANY_LINKS;
    case ENOENT:
    case ENOTDIR:
    case EIO: return REAP_RET_NOT_FOUND;
    case ENOMEM: return REAP_RET_OUT_OF_MEMORY;
    default: return REAP_RET_OTHER;
    }
}
