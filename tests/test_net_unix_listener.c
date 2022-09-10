#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <reap/reap.h>

int
main(int argc, char **argv)
{
    int ret, listener;
    socklen_t slen;
    struct sockaddr_un addr = {.sun_family = AF_LOCAL};
    struct stat fs;
    reapNetIterator *iterator;
    reapNetResult result;

    if (argc < 2) {
        return REAP_RET_BAD_USAGE;
    }

    addr.sun_path[0] = '\0';
    snprintf(&addr.sun_path[1], sizeof(addr.sun_path) - 1, "%s", argv[1]);
    slen = offsetof(struct sockaddr_un, sun_path) + 1 + strlen(&addr.sun_path[1]);

    listener = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (listener < 0) {
        return -errno;
    }

    if (fstat(listener, &fs) != 0) {
        ret = -errno;
        goto done;
    }
    printf("%lu\n", (unsigned long)fs.st_ino);

    if (bind(listener, (struct sockaddr *)&addr, slen) != 0) {
        ret = -errno;
        goto done;
    }

    ret = reapNetIteratorCreate(REAP_NET_FLAG_DOMAIN, &iterator);
    if (ret != REAP_RET_OK) {
        goto done;
    }

    while ((ret = reapNetIteratorNext(iterator, &result)) == REAP_RET_OK) {
        if (result.socket_type == SOCK_STREAM && !result.connected) {
            printf("%lu %s\n", (unsigned long)result.inode, result.path);
        }
    }

    reapNetIteratorDestroy(iterator);

    if (ret == REAP_RET_DONE) {
        ret = REAP_RET_OK;
    }

done:
    close(listener);
    return ret;
}
