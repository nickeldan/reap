#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <scrutiny/scrutiny.h>

#include <reap/reap.h>

struct serverArgs {
    int listener;
    char path[STRUCT_FIELD_SIZE(struct sockaddr_un, sun_path)];
};

void *
domainServerSetup(void *global_ctx)
{
    struct sockaddr_un addr = {.sun_family = AF_LOCAL};
    struct serverArgs *args;

    (void)global_ctx;

    SCR_ASSERT_PTR_NEQ((args = malloc(sizeof(*args))), NULL);

    args->listener = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (args->listener < 0) {
        SCR_FAIL("socket: %s", strerror(errno));
    }

    snprintf(addr.sun_path, sizeof(addr.sun_path), "/tmp/reap_test_%li", (long)getpid());
    strncpy(args->path, addr.sun_path, sizeof(args->path));
    if (bind(args->listener, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        SCR_FAIL("bind: %s", reapGetError());
    }

    if (listen(args->listener, 1) != 0) {
        SCR_FAIL("listen (%s): %s", addr.sun_path, strerror(errno));
    }

    return args;
}

void
domainServerCleanup(void *group_ctx)
{
    struct serverArgs *args = group_ctx;

    close(args->listener);
    unlink(args->path);
    free(args);
}

void
domainFindServer(void)
{
    int ret;
    struct stat fs;
    struct serverArgs *args;
    reapNetIterator *iterator;
    reapNetResult result;

    args = SCR_GROUP_CTX();

    if (fstat(args->listener, &fs) != 0) {
        SCR_FAIL("fstat: %s", strerror(errno));
    }

    if (reapNetIteratorCreate(REAP_NET_FLAG_DOMAIN, &iterator) != REAP_RET_OK) {
        SCR_FAIL("reapNetIteratorCreate: %s", reapGetError());
    }

    while ((ret = reapNetIteratorNext(iterator, &result)) == REAP_RET_OK) {
        SCR_ASSERT_EQ(result.flags, REAP_NET_FLAG_DOMAIN);

        if (result.inode == fs.st_ino) {
            SCR_ASSERT_EQ(result.socket_type, SOCK_STREAM);
            SCR_ASSERT_EQ(result.connected, 0);
            SCR_ASSERT_STR_EQ(result.path, args->path);

            reapNetIteratorDestroy(iterator);
            return;
        }
    }

    if (ret != REAP_RET_DONE) {
        SCR_FAIL("reapNetIteratorNext: %s", reapGetError());
    }

    SCR_FAIL("Could not find entry for server");
}