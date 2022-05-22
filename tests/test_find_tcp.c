#include <arpa/inet.h>
#include <stdio.h>

#include <reap/reap.h>

int
main()
{
    int ret;
    reapNetIterator iterator;
    reapNetResult result;

    ret = reapNetIteratorInit(&iterator, true);
    if (ret != REAP_RET_OK) {
        fprintf(stderr, "reapNetIteratorInit: %s\n",
#ifdef REAP_USE_ERROR_BUFFER
                reapGetError()
#else
                reapErrorString(ret)
#endif
        );
        return ret;
    }

    while ((ret = reapNetIteratorNext(&iterator, &result)) == REAP_RET_OK) {
        char local_buffer[INET_ADDRSTRLEN], remote_buffer[INET_ADDRSTRLEN];

        inet_ntop(AF_INET, &result.local.sin_addr, local_buffer, sizeof(local_buffer));

        printf("inode %lu: ", (unsigned long)result.inode);
        if (result.remote.sin_port == 0) {
            printf("Listening on %s:%u\n", local_buffer, result.local.sin_port);
        }
        else {
            inet_ntop(AF_INET, &result.remote.sin_addr, remote_buffer, sizeof(remote_buffer));
            printf("%s:%u connected to %s:%u\n", local_buffer, result.local.sin_port, remote_buffer,
                   result.remote.sin_port);
        }
    }

    reapNetIteratorClose(&iterator);

    if (ret == REAP_RET_DONE) {
        ret = REAP_RET_OK;
    }
    else {
        fprintf(stderr, "reapNetIteratorNext: %s\n",
#ifdef REAP_USE_ERROR_BUFFER
                reapGetError()
#else
                reapErrorString(ret)
#endif
        );
    }

    return ret;
}
