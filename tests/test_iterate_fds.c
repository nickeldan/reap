#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <scrutiny/scrutiny.h>

#include <reap/reap.h>

void *
fdSetup(void *global_ctx)
{
    int fd;
    char *template;

    (void)global_ctx;

    SCR_ASSERT_PTR_NEQ((template = malloc(25)), NULL);
    snprintf(template, 25, "/tmp/reap_test_XXXXXX");
    fd = mkstemp(template);
    if (fd < 0) {
        SCR_ERROR("mkstemp: %s", strerror(errno));
    }
    close(fd);
    return template;
}

void
fdCleanup(void *group_ctx)
{
    unlink((char *)group_ctx);
    free(group_ctx);
}

void
iterateFds(void)
{
    int fd, ret;
    struct stat fs;
    reapFdIterator *iterator;
    reapFdResult result;
    char *tmp_file;
    char path[PATH_MAX];

    tmp_file = SCR_GROUP_CTX();
    fd = open(tmp_file, O_RDONLY);
    if (fd < 0) {
        SCR_ERROR("open (%s): %s", tmp_file, strerror(errno));
    }

    if (fstat(fd, &fs) != 0) {
        SCR_ERROR("fstat: %s", strerror(errno));
    }

    SCR_ASSERT_EQ(reapFdIteratorCreate(getpid(), &iterator), REAP_RET_OK);

    while ((ret = reapFdIteratorNext(iterator, &result, path, sizeof(path))) == REAP_RET_OK) {
        SCR_LOG("%i: %s", result.fd, path);

        if (result.fd == fd) {
            SCR_ASSERT_EQ(result.inode, fs.st_ino);
            SCR_ASSERT_EQ(result.device, fs.st_dev);
            SCR_ASSERT_STR_EQ(path, tmp_file);

            close(fd);
            reapFdIteratorDestroy(iterator);
            return;
        }
    }

    if (ret != REAP_RET_DONE) {
        SCR_ERROR("reapFdIteratorNext: %s", reapGetError());
    }

    SCR_ERROR("Could not find file descriptor %i", fd);
}
