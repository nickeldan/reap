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

static void
clientLogic(const struct sockaddr *addr, socklen_t slen)
{
    int client;
    unsigned char dummy;

    alarm(5);

    client = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (client < 0) {
        return;
    }

    if (connect(client, addr, slen) != 0 || read(client, &dummy, 1) < 0) {}

    close(client);
}

int
main(int argc, char **argv)
{
    int ret, listener, peer;
    unsigned char dummy = 0;
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

    if (listen(listener, 1) != 0) {
        ret = -errno;
        goto done;
    }

    switch (fork()) {
    case -1: ret = -errno; goto done;

    case 0:
        close(listener);
        clientLogic((struct sockaddr *)&addr, slen);
        return 0;

    default: break;
    }

    alarm(5);

    peer = accept(listener, (struct sockaddr *)&addr, &slen);
    if (peer < 0) {
        ret = -errno;
        goto done;
    }

    ret = reapNetIteratorCreate(REAP_NET_FLAG_DOMAIN, &iterator);
    if (ret != REAP_RET_OK) {
        goto done_close_peer;
    }

    while ((ret = reapNetIteratorNext(iterator, &result)) == REAP_RET_OK) {
        if (result.socket_type == SOCK_STREAM) {
            printf("%lu %u %s\n", (unsigned long)result.inode, result.connected, result.path);
        }
    }

    if (write(peer, &dummy, 1) < 0) {}

    reapNetIteratorDestroy(iterator);

    if (ret == REAP_RET_DONE) {
        ret = REAP_RET_OK;
    }

done_close_peer:
    close(peer);

done:
    close(listener);
    return ret;
}
