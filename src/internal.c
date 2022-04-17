#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "internal.h"

int
translateErrno(int errno_value)
{
    switch (errno_value) {
    case EINVAL: return REAP_RET_BAD_USAGE;
    case EACCES: return REAP_RET_NO_PERMISSION;
    case ELOOP: return REAP_RET_TOO_MANY_LINKS;
    case ENOENT:
    case ENOTDIR:
    case EIO: return REAP_RET_NOT_FOUND;
    case ENOMEM: return REAP_RET_OUT_OF_MEMORY;
    default: return REAP_RET_OTHER;
    }
}

int
betterReadlink(const char *pathname, char *buf, size_t bufsiz)
{
    int ret;

    memset(buf, '\0', bufsiz);
    ret = readlink(pathname, buf, bufsiz);
    buf[bufsiz - 1] = '\0';
    return ret;
}
