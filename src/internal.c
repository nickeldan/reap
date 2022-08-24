#include <string.h>
#include <unistd.h>

#include "internal.h"

int
betterReadlink(const char *pathname, char *buf, size_t bufsiz)
{
    int ret;

    memset(buf, '\0', bufsiz);
    ret = readlink(pathname, buf, bufsiz);
    buf[bufsiz - 1] = '\0';
    return ret;
}
