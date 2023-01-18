#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <pthread.h>
#include <semaphore.h>

#include <scrutiny/scrutiny.h>

#include <reap/reap.h>

#define PORT 13370

struct threadArgs {
    int listener;
    sem_t sem;
};

static void *
serverFunc(void *opaque)
{
    int peer;
    struct threadArgs *args = opaque;
    struct sockaddr_storage addr;
    socklen_t slen = sizeof(addr);

    peer = accept(args->listener, (struct sockaddr *)&addr, &slen);
    if (peer < 0) {
        SCR_ERROR("accept: %s", strerror(errno));
    }

    sem_wait(&args->sem);
    close(peer);

    return NULL;
}

void
socketCleanup(void *group_ctx)
{
    int listener = (intptr_t)group_ctx;

    shutdown(listener, SHUT_RDWR);
    close(listener);
}

void *
tcpIpv4Setup(void *global_ctx)
{
    int listener, dummy = 1;
    struct sockaddr_in addr = {.sin_family = AF_INET};

    (void)global_ctx;

    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(PORT);

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) {
        SCR_ERROR("socket: %s", strerror(errno));
    }

    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &dummy, sizeof(dummy)) < 0) {
        SCR_ERROR("setsockopt (SO_REUSEADDR): %s", strerror(errno));
    }

    if (setsockopt(listener, SOL_SOCKET, SO_REUSEPORT, &dummy, sizeof(dummy)) < 0) {
        SCR_ERROR("setsockopt (SO_REUSEADDR): %s", strerror(errno));
    }

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        SCR_ERROR("bind: %s", strerror(errno));
    }

    if (listen(listener, 1) != 0) {
        int local_errno = errno;

        shutdown(listener, SHUT_RDWR);
        close(listener);
        SCR_ERROR("listen: %s", strerror(local_errno));
    }

    return (void *)(intptr_t)listener;
}

void *
tcpIpv6Setup(void *global_ctx)
{
    int listener, dummy = 1;
    struct sockaddr_in6 addr = {.sin6_family = AF_INET6, .sin6_addr = IN6ADDR_LOOPBACK_INIT};

    (void)global_ctx;

    addr.sin6_port = htons(PORT);

    listener = socket(AF_INET6, SOCK_STREAM, 0);
    if (listener < 0) {
        SCR_ERROR("socket: %s", strerror(errno));
    }

    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &dummy, sizeof(dummy)) < 0) {
        SCR_ERROR("setsockopt (SO_REUSEADDR): %s", strerror(errno));
    }

    if (setsockopt(listener, SOL_SOCKET, SO_REUSEPORT, &dummy, sizeof(dummy)) < 0) {
        SCR_ERROR("setsockopt (SO_REUSEADDR): %s", strerror(errno));
    }

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        SCR_ERROR("bind: %s", strerror(errno));
    }

    if (listen(listener, 1) != 0) {
        int local_errno = errno;

        shutdown(listener, SHUT_RDWR);
        close(listener);
        SCR_ERROR("listen: %s", strerror(local_errno));
    }

    return (void *)(intptr_t)listener;
}

void
iterate_tcp_ipv4_find_server(void)
{
    int ret, listener;
    struct stat fs;
    reapNetIterator *iterator;
    reapNetResult result;

    listener = (intptr_t)SCR_GROUP_CTX();

    if (fstat(listener, &fs) != 0) {
        SCR_ERROR("fstat: %s", strerror(errno));
    }

    SCR_ASSERT_EQ(reapNetIteratorCreate(0, &iterator), REAP_RET_OK);

    while ((ret = reapNetIteratorNext(iterator, &result)) == REAP_RET_OK) {
        SCR_ASSERT_EQ(result.flags, 0);

        if (result.local.port == PORT) {
            const unsigned char loopback_address[4] = {127, 0, 0, 1}, zero_address[4] = {0};

            SCR_ASSERT_EQ(result.inode, fs.st_ino);

            SCR_ASSERT_MEM_EQ(result.local.address, loopback_address, sizeof(loopback_address));

            SCR_ASSERT_EQ(result.remote.port, 0);
            SCR_ASSERT_MEM_EQ(result.remote.address, zero_address, sizeof(zero_address));

            reapNetIteratorDestroy(iterator);
            return;
        }
    }

    if (ret != REAP_RET_DONE) {
        SCR_ERROR("reapNetIteratorNext: %s", reapGetError());
    }

    SCR_ERROR("Could not find entry for server");
}

void
iterate_tcp_ipv4_find_client(void)
{
    int ret, client;
    pthread_t thread;
    struct sockaddr_in addr = {.sin_family = AF_INET};
    struct stat fs;
    reapNetIterator *iterator;
    reapNetResult result;
    struct threadArgs args;

    client = socket(AF_INET, SOCK_STREAM, 0);
    if (client < 0) {
        SCR_ERROR("socket: %s", strerror(errno));
    }

    if (fstat(client, &fs) != 0) {
        SCR_ERROR("fstat: %s", strerror(errno));
    }

    args.listener = (intptr_t)SCR_GROUP_CTX();
    if (sem_init(&args.sem, 0, 0) != 0) {
        SCR_ERROR("sem_init: %s", strerror(errno));
    }

    ret = pthread_create(&thread, NULL, serverFunc, &args);
    if (ret != 0) {
        SCR_ERROR("pthread_create: %s", strerror(ret));
    }

    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(PORT);

    if (connect(client, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        SCR_ERROR("connect: %s", strerror(errno));
    }

    SCR_ASSERT_EQ(reapNetIteratorCreate(0, &iterator), REAP_RET_OK);

    while ((ret = reapNetIteratorNext(iterator, &result)) == REAP_RET_OK) {
        SCR_ASSERT_EQ(result.flags, 0);

        if (result.remote.port == PORT && result.remote.address[0] == 127 && result.remote.address[1] == 0 &&
            result.remote.address[2] == 0 && result.remote.address[3] == 1) {
            SCR_ASSERT_EQ(result.inode, fs.st_ino);

            reapNetIteratorDestroy(iterator);
            sem_post(&args.sem);
            pthread_join(thread, NULL);
            shutdown(client, SHUT_RDWR);
            close(client);
            return;
        }
    }

    if (ret != REAP_RET_DONE) {
        SCR_ERROR("reapNetIteratorNext: %s", reapGetError());
    }

    SCR_ERROR("Could not find entry for client");
}

void
iterate_tcp_ipv6_find_server(void)
{
    int ret, listener;
    struct stat fs;
    reapNetIterator *iterator;
    reapNetResult result;

    listener = (intptr_t)SCR_GROUP_CTX();

    if (fstat(listener, &fs) != 0) {
        SCR_ERROR("fstat: %s", strerror(errno));
    }

    SCR_ASSERT_EQ(reapNetIteratorCreate(REAP_NET_FLAG_IPV6, &iterator), REAP_RET_OK);

    while ((ret = reapNetIteratorNext(iterator, &result)) == REAP_RET_OK) {
        SCR_ASSERT_EQ(result.flags, REAP_NET_FLAG_IPV6);

        if (result.local.port == PORT) {
            const unsigned char loopback_address[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
                                zero_address[16] = {0};

            SCR_ASSERT_EQ(result.inode, fs.st_ino);

            SCR_ASSERT_MEM_EQ(result.local.address, loopback_address, sizeof(loopback_address));

            SCR_ASSERT_EQ(result.remote.port, 0);
            SCR_ASSERT_MEM_EQ(result.remote.address, zero_address, sizeof(zero_address));

            reapNetIteratorDestroy(iterator);
            return;
        }
    }

    if (ret != REAP_RET_DONE) {
        SCR_ERROR("reapNetIteratorNext: %s", reapGetError());
    }

    SCR_ERROR("Could not find entry for server");
}

void
iterate_tcp_ipv6_find_client(void)
{
    int ret, client;
    pthread_t thread;
    struct sockaddr_in6 addr = {.sin6_family = AF_INET6, .sin6_addr = IN6ADDR_LOOPBACK_INIT};
    struct stat fs;
    reapNetIterator *iterator;
    reapNetResult result;
    struct threadArgs args;
    const unsigned char loopback_address[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};

    client = socket(AF_INET6, SOCK_STREAM, 0);
    if (client < 0) {
        SCR_ERROR("socket: %s", strerror(errno));
    }

    if (fstat(client, &fs) != 0) {
        SCR_ERROR("fstat: %s", strerror(errno));
    }

    args.listener = (intptr_t)SCR_GROUP_CTX();
    if (sem_init(&args.sem, 0, 0) != 0) {
        SCR_ERROR("sem_init: %s", strerror(errno));
    }

    ret = pthread_create(&thread, NULL, serverFunc, &args);
    if (ret != 0) {
        SCR_ERROR("pthread_create: %s", strerror(ret));
    }

    addr.sin6_port = htons(PORT);

    if (connect(client, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        SCR_ERROR("connect: %s", strerror(errno));
    }

    SCR_ASSERT_EQ(reapNetIteratorCreate(REAP_NET_FLAG_IPV6, &iterator), REAP_RET_OK);

    while ((ret = reapNetIteratorNext(iterator, &result)) == REAP_RET_OK) {
        SCR_ASSERT_EQ(result.flags, REAP_NET_FLAG_IPV6);

        if (result.remote.port == PORT &&
            memcmp(result.remote.address, loopback_address, sizeof(loopback_address)) == 0) {
            SCR_ASSERT_EQ(result.inode, fs.st_ino);

            reapNetIteratorDestroy(iterator);
            sem_post(&args.sem);
            pthread_join(thread, NULL);
            shutdown(client, SHUT_RDWR);
            close(client);
            return;
        }
    }

    if (ret != REAP_RET_DONE) {
        SCR_ERROR("reapNetIteratorNext: %s", reapGetError());
    }

    SCR_ERROR("Could not find entry for client");
}
