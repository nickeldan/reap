#include <arpa/inet.h>
#include <stdio.h>

#include <reap/iterate_net.h>

#include "common.h"

static int
showResults(bool tcp)
{
    int ret;
    reapNetIterator iterator;
    reapNetResult result;

    ret = reapNetIteratorInit(&iterator, tcp);
    if (ret != REAP_RET_OK) {
        fprintf(stderr, "reapNetIteratorInit: %s\n", ERROR(ret));
        return ret;
    }

    while ((ret = reapNetIteratorNext(&iterator, &result)) == REAP_RET_OK) {
        char local_buffer[INET_ADDRSTRLEN], remote_buffer[INET_ADDRSTRLEN];

        inet_ntop(AF_INET, &result.local.address, local_buffer, sizeof(local_buffer));

        printf("(%lu) ", (unsigned long)result.inode);
        if (result.remote.port == 0) {
            printf("Listening on %s:%u\n", local_buffer, result.local.port);
        }
        else {
            inet_ntop(AF_INET, &result.remote.address, remote_buffer, sizeof(remote_buffer));
            printf("%s:%u communicating with %s:%u\n", local_buffer, result.local.port, remote_buffer,
                   result.remote.port);
        }
    }

    reapNetIteratorClose(&iterator);

    if (ret == REAP_RET_DONE) {
        ret = REAP_RET_OK;
    }
    else {
        fprintf(stderr, "reapNetIteratorNext: %s\n", ERROR(ret));
    }

    return ret;
}

static int
showResults6(bool tcp)
{
    int ret;
    reapNet6Iterator iterator;
    reapNet6Result result;

    ret = reapNet6IteratorInit(&iterator, tcp);
    if (ret != REAP_RET_OK) {
        fprintf(stderr, "reapNetIterator6Init: %s\n", ERROR(ret));
        return ret;
    }

    while ((ret = reapNet6IteratorNext(&iterator, &result)) == REAP_RET_OK) {
        char local_buffer[INET6_ADDRSTRLEN], remote_buffer[INET6_ADDRSTRLEN];

        inet_ntop(AF_INET6, &result.local.address, local_buffer, sizeof(local_buffer));

        printf("(%lu) ", (unsigned long)result.inode);
        if (result.remote.port == 0) {
            printf("Listening on [%s]:%u\n", local_buffer, result.local.port);
        }
        else {
            inet_ntop(AF_INET6, &result.remote.address, remote_buffer, sizeof(remote_buffer));
            printf("%s:%u communicating with [%s]:%u\n", local_buffer, result.local.port, remote_buffer,
                   result.remote.port);
        }
    }

    reapNet6IteratorClose(&iterator);

    if (ret == REAP_RET_DONE) {
        ret = REAP_RET_OK;
    }
    else {
        fprintf(stderr, "reapNetIteratorNext: %s\n", ERROR(ret));
    }

    return ret;
}

int
main()
{
    int ret;

    printf("TCP:\n");
    ret = showResults(true);
    if (ret != REAP_RET_OK) {
        return ret;
    }
    ret = showResults6(true);
    if (ret != REAP_RET_OK) {
        return ret;
    }

    printf("\nUDP:\n");
    ret = showResults(false);
    if (ret != REAP_RET_OK) {
        return ret;
    }
    return showResults6(false);
}
