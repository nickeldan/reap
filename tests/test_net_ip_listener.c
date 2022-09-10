#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <reap/reap.h>

#define PORT 5000

int
main(int argc, char **argv)
{
    int ret, domain, type, listener;
    unsigned int flags = 0;
    socklen_t slen;
    const char *format;
    union {
        struct sockaddr_in v4;
        struct sockaddr_in6 v6;
    } addr;
    struct stat fs;
    reapNetIterator *iterator;
    reapNetResult result;

    if (argc < 3) {
        return REAP_RET_BAD_USAGE;
    }

    if (argv[1][0] == '1') {
        flags |= REAP_NET_FLAG_IPV6;
        addr.v6.sin6_family = domain = AF_INET6;
        memset(addr.v6.sin6_addr.s6_addr, 0, IPV6_SIZE);
        addr.v6.sin6_addr.s6_addr[IPV6_SIZE - 1] = 1;
        addr.v6.sin6_port = htons(PORT);
        slen = sizeof(addr.v6);
        format = "%lu [%s]:%u -> [%s]:%u\n";
    }
    else {
        addr.v4.sin_family = domain = AF_INET;
        addr.v4.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        addr.v4.sin_port = htons(PORT);
        slen = sizeof(addr.v4);
        format = "%lu %s:%u -> %s:%u\n";
    }

    if (argv[2][0] == '1') {
        flags |= REAP_NET_FLAG_UDP;
        type = SOCK_DGRAM;
    }
    else {
        type = SOCK_STREAM;
    }

    listener = socket(domain, type, 0);
    if (listener < 0) {
        ret = -errno;
        goto done;
    }

    if (bind(listener, (struct sockaddr *)&addr, slen) != 0) {
        ret = -errno;
        goto done;
    }

    if (type == SOCK_STREAM && listen(listener, 1) != 0) {
        ret = -errno;
        goto done;
    }

    if (fstat(listener, &fs) != 0) {
        ret = -errno;
        goto done;
    }
    printf("%lu\n", (unsigned long)fs.st_ino);

    ret = reapNetIteratorCreate(flags, &iterator);
    if (ret != REAP_RET_OK) {
        goto done;
    }

    while ((ret = reapNetIteratorNext(iterator, &result)) == REAP_RET_OK) {
        char local_buffer[INET6_ADDRSTRLEN], remote_buffer[INET6_ADDRSTRLEN];

        inet_ntop(domain, result.local.address, local_buffer, sizeof(local_buffer));
        inet_ntop(domain, result.remote.address, remote_buffer, sizeof(remote_buffer));

        printf(format, (unsigned long)result.inode, local_buffer, result.local.port, remote_buffer,
               result.remote.port);
    }

    reapNetIteratorDestroy(iterator);

    if (ret == REAP_RET_DONE) {
        ret = REAP_RET_OK;
    }

done:
    close(listener);
    return ret;
}
