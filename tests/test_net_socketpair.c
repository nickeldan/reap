#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <reap/reap.h>

int
main()
{
    int ret;
    int fds[2];
    reapNetIterator *iterator;
    reapNetResult result;

    if (socketpair(AF_LOCAL, SOCK_STREAM, 0, fds) != 0) {
        return -errno;
    }

    for (int k = 0; k < 2; k++) {
        struct stat fs;

        if (fstat(fds[k], &fs) != 0) {
            ret = -errno;
            goto done;
        }
        printf("%lu\n", (unsigned long)fs.st_ino);
    }

    ret = reapNetIteratorCreate(REAP_NET_FLAG_DOMAIN, &iterator);
    if (ret != REAP_RET_OK) {
        return ret;
    }

    while ((ret = reapNetIteratorNext(iterator, &result)) == REAP_RET_OK) {
        if (result.socket_type == SOCK_STREAM && result.connected) {
            printf("%lu\n", (unsigned long)result.inode);
        }
    }

    reapNetIteratorDestroy(iterator);

    if (ret == REAP_RET_DONE) {
        ret = REAP_RET_OK;
    }

done:
    close(fds[0]);
    close(fds[1]);
    return ret;
}
