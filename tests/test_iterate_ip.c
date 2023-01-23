#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdbool.h>
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

static const unsigned char loopback_addr[4] = {127, 0, 0, 1}, zero_addr[4] = {0},
                           loopback_addr6[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
                           zero_addr6[16] = {0};

static void
bindServer(int listener, const struct sockaddr *addr, socklen_t slen, bool tcp)
{
    int dummy = 1;

    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &dummy, sizeof(dummy)) < 0) {
        SCR_ERROR("setsockopt (SO_REUSEADDR): %s", strerror(errno));
    }

    if (setsockopt(listener, SOL_SOCKET, SO_REUSEPORT, &dummy, sizeof(dummy)) < 0) {
        SCR_ERROR("setsockopt (SO_REUSEPORT): %s", strerror(errno));
    }

    if (bind(listener, addr, slen) != 0) {
        int local_errno = errno;

        // At the moment, I can't get IPv6 tests to work in my GitHub Action.
        if (addr->sa_family == AF_INET6 && local_errno == EADDRNOTAVAIL) {
            SCR_TEST_SKIP();
        }

        SCR_ERROR("bind: %s", strerror(local_errno));
    }

    if (tcp && listen(listener, 1) != 0) {
        int local_errno = errno;

        shutdown(listener, SHUT_RDWR);
        close(listener);
        SCR_ERROR("listen: %s", strerror(local_errno));
    }
}

static int
ipv4ServerSetup(int type)
{
    int listener;
    struct sockaddr_in addr = {.sin_family = AF_INET};

    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(PORT);

    listener = socket(AF_INET, type, 0);
    if (listener < 0) {
        SCR_ERROR("socket: %s", strerror(errno));
    }

    bindServer(listener, (struct sockaddr *)&addr, sizeof(addr), type == SOCK_STREAM);

    return listener;
}

static int
ipv6ServerSetup(int type)
{
    int listener;
    struct sockaddr_in6 addr = {.sin6_family = AF_INET6, .sin6_addr = IN6ADDR_LOOPBACK_INIT};

    addr.sin6_port = htons(PORT);

    listener = socket(AF_INET6, type, 0);
    if (listener < 0) {
        SCR_ERROR("socket: %s", strerror(errno));
    }

    bindServer(listener, (struct sockaddr *)&addr, sizeof(addr), type == SOCK_STREAM);

    return listener;
}

static void
ipv4FindServer(bool udp)
{
    int ret, listener;
    unsigned int flags = udp * REAP_NET_FLAG_UDP;
    struct stat fs;
    reapNetIterator *iterator;
    reapNetResult result;

    listener = (intptr_t)SCR_GROUP_CTX();

    if (fstat(listener, &fs) != 0) {
        SCR_ERROR("fstat: %s", strerror(errno));
    }

    if (reapNetIteratorCreate(flags, &iterator) != REAP_RET_OK) {
        SCR_ERROR("reapNetIteratorCreate: %s", reapGetError());
    }

    while ((ret = reapNetIteratorNext(iterator, &result)) == REAP_RET_OK) {
        SCR_ASSERT_EQ(result.flags, flags);

        if (result.inode == fs.st_ino) {
            SCR_ASSERT_EQ(result.local.port, PORT);
            SCR_ASSERT_MEM_EQ(result.local.address, loopback_addr, sizeof(loopback_addr));

            SCR_ASSERT_EQ(result.remote.port, 0);
            SCR_ASSERT_MEM_EQ(result.remote.address, zero_addr, sizeof(zero_addr));

            reapNetIteratorDestroy(iterator);
            return;
        }
    }

    if (ret != REAP_RET_DONE) {
        SCR_ERROR("reapNetIteratorNext: %s", reapGetError());
    }

    SCR_ERROR("Could not find entry for server");
}

static void
ipv6FindServer(bool udp)
{
    int ret, listener;
    unsigned int flags = REAP_NET_FLAG_IPV6 | (udp * REAP_NET_FLAG_UDP);
    struct stat fs;
    reapNetIterator *iterator;
    reapNetResult result;

    listener = (intptr_t)SCR_GROUP_CTX();

    if (fstat(listener, &fs) != 0) {
        SCR_ERROR("fstat: %s", strerror(errno));
    }

    if (reapNetIteratorCreate(flags, &iterator) != REAP_RET_OK) {
        SCR_ERROR("reapNetIteratorCreate: %s", reapGetError());
    }

    while ((ret = reapNetIteratorNext(iterator, &result)) == REAP_RET_OK) {
        SCR_ASSERT_EQ(result.flags, flags);

        if (result.inode == fs.st_ino) {
            SCR_ASSERT_EQ(result.local.port, PORT);

            SCR_ASSERT_MEM_EQ(result.local.address, loopback_addr6, sizeof(loopback_addr6));

            SCR_ASSERT_EQ(result.remote.port, 0);
            SCR_ASSERT_MEM_EQ(result.remote.address, zero_addr6, sizeof(zero_addr6));

            reapNetIteratorDestroy(iterator);
            return;
        }
    }

    if (ret != REAP_RET_DONE) {
        SCR_ERROR("reapNetIteratorNext: %s", reapGetError());
    }

    SCR_ERROR("Could not find entry for server");
}

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

void *
ipv4TcpServerSetup(void *global_ctx)
{
    (void)global_ctx;

    return (void *)(intptr_t)ipv4ServerSetup(SOCK_STREAM);
}

void *
ipv4UdpServerSetup(void *global_ctx)
{
    (void)global_ctx;

    return (void *)(intptr_t)ipv4ServerSetup(SOCK_DGRAM);
}

void *
ipv6TcpServerSetup(void *global_ctx)
{
    (void)global_ctx;

    return (void *)(intptr_t)ipv6ServerSetup(SOCK_STREAM);
}

void *
ipv6UdpServerSetup(void *global_ctx)
{
    (void)global_ctx;

    return (void *)(intptr_t)ipv6ServerSetup(SOCK_DGRAM);
}

void
serverCleanup(void *group_ctx)
{
    int listener = (intptr_t)group_ctx;

    shutdown(listener, SHUT_RDWR);
    close(listener);
}

void
ipv4TcpFindServer(void)
{
    ipv4FindServer(false);
}

void
ipv4TcpFindClient(void)
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

    if (reapNetIteratorCreate(0, &iterator) != REAP_RET_OK) {
        SCR_ERROR("reapNetIteratorCreate: %s", reapGetError());
    }

    while ((ret = reapNetIteratorNext(iterator, &result)) == REAP_RET_OK) {
        SCR_ASSERT_EQ(result.flags, 0);

        if (result.inode == fs.st_ino) {
            SCR_ASSERT_EQ(result.remote.port, PORT);
            SCR_ASSERT_MEM_EQ(result.remote.address, loopback_addr, sizeof(loopback_addr));

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
ipv6TcpFindServer(void)
{
    ipv6FindServer(false);
}

void
ipv6TcpFindClient(void)
{
    int ret, client;
    pthread_t thread;
    struct sockaddr_in6 addr = {.sin6_family = AF_INET6, .sin6_addr = IN6ADDR_LOOPBACK_INIT};
    struct stat fs;
    reapNetIterator *iterator;
    reapNetResult result;
    struct threadArgs args;

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

    if (reapNetIteratorCreate(REAP_NET_FLAG_IPV6, &iterator) != REAP_RET_OK) {
        SCR_ERROR("reapNetIteratorCreate: %s", reapGetError());
    }

    while ((ret = reapNetIteratorNext(iterator, &result)) == REAP_RET_OK) {
        SCR_ASSERT_EQ(result.flags, REAP_NET_FLAG_IPV6);

        if (result.inode == fs.st_ino) {
            SCR_ASSERT_EQ(result.remote.port, PORT);
            SCR_ASSERT_MEM_EQ(result.remote.address, loopback_addr6, sizeof(loopback_addr6));

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
ipv4UdpFindServer(void)
{
    ipv4FindServer(true);
}

void
ipv6UdpFindServer(void)
{
    ipv6FindServer(true);
}
