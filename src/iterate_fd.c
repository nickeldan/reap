#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <reap/iterate_fd.h>

#include "internal.h"

int
reapFdIteratorInit(pid_t pid, reapFdIterator *iterator)
{
    char buffer[100];

    if (pid <= 0 || !iterator) {
        return REAP_RET_BAD_USAGE;
    }

    iterator->pid = pid;
    snprintf(buffer, sizeof(buffer), "/proc/%li/fd", (long)pid);
    iterator->dir = opendir(buffer);
    return iterator->dir ? REAP_RET_OK : translateErrno();
}

void
reapFdIteratorClose(reapFdIterator *iterator)
{
    if (iterator && iterator->dir) {
        closedir(iterator->dir);
        iterator->dir = NULL;
    }
}

int
reapFdIteratorNext(const reapFdIterator *iterator, reapFdResult *result)
{
    long value;
    char *endptr;
    char buffer[100];
    struct dirent *entry;

    if (!iterator || !iterator->dir || !result) {
        return REAP_RET_BAD_USAGE;
    }

    errno = 0;
    entry = readdir(iterator->dir);
    if (!entry) {
        return (errno == 0) ? REAP_RET_DONE : translateErrno();
    }

    value = strtol(entry->d_name, &endptr, 10);
    if (*endptr != '\0' || value < 0 || value > INT_MAX) {
        return REAP_RET_OTHER;
    }

    result->fd = value;

    snprintf(buffer, sizeof(buffer), "/proc/%li/fd/%i", (long)iterator->pid, result->fd);
    return (betterReadlink(buffer, result->file, sizeof(result->file)) == 0) ? REAP_RET_OK :
                                                                               translateErrno();
}

int
reapReadFd(pid_t pid, int fd, char *dest, size_t size)
{
    char buffer[100];

    if (pid <= 0 || fd < 0 || !dest) {
        return REAP_RET_BAD_USAGE;
    }

    snprintf(buffer, sizeof(buffer), "/proc/%li/fd/%i", (long)pid, fd);
    return (betterReadlink(buffer, dest, size) == 0) ? REAP_RET_OK : translateErrno();
}
