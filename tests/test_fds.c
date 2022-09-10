#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <reap/reap.h>

int
main(int argc, char **argv)
{
    int ret, fd;
    char path[PATH_MAX];
    reapFdIterator *iterator;
    reapFdResult result;

    if (argc < 2) {
        return REAP_RET_BAD_USAGE;
    }

    fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        return -1 * errno;
    }

    ret = reapFdIteratorCreate(getpid(), &iterator);
    if (ret != REAP_RET_OK) {
        goto done;
    }

    while ((ret = reapFdIteratorNext(iterator, &result, path, sizeof(path))) == REAP_RET_OK) {
        printf("%i %u %lu %s\n", result.fd, (unsigned int)result.device, (unsigned long)result.inode, path);
    }

    reapFdIteratorDestroy(iterator);

    if (ret == REAP_RET_DONE) {
        ret = REAP_RET_OK;
    }

done:
    close(fd);
    return ret;
}
