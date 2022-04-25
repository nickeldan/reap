#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <reap/iterate_fd.h>

#include "internal.h"

int
reapFdIteratorInit(pid_t pid, reapFdIterator *iterator)
{
    char buffer[100];

    if (pid <= 0 || !iterator) {
        if (pid <= 0) {
            EMIT_ERROR("The PID must be positive");
        }
        else {
            EMIT_ERROR("The iterator cannot be NULL");
        }
        return REAP_RET_BAD_USAGE;
    }

    iterator->pid = pid;
    snprintf(buffer, sizeof(buffer), "/proc/%li/fd", (long)pid);
    iterator->dir = opendir(buffer);
    if (!iterator->dir) {
        int local_errno = errno;

        EMIT_ERROR("opendir failed: %s", strerror(local_errno));
        return translateErrno(local_errno);
    }

    return REAP_RET_OK;
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
    struct stat fs;

    if (!iterator || !iterator->dir || !result) {
        if (!iterator) {
            EMIT_ERROR("The iterator cannot be NULL");
        }
        else if (!iterator->dir) {
            EMIT_ERROR("This iterator has been closed");
        }
        else {
            EMIT_ERROR("The result cannot be NULL");
        }
        return REAP_RET_BAD_USAGE;
    }

    do {
        errno = 0;
        entry = readdir(iterator->dir);
        if (!entry) {
            int local_errno = errno;

            if (local_errno == 0) {
                return REAP_RET_DONE;
            }
            else {
                EMIT_ERROR("readdir failed: %s", strerror(local_errno));
                return translateErrno(local_errno);
            }
        }
    } while (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0);

    value = strtol(entry->d_name, &endptr, 10);
    if (*endptr != '\0' || value < 0 || value > INT_MAX) {
        EMIT_ERROR("Invalid file in fd directory: %s", entry->d_name);
        return REAP_RET_OTHER;
    }

    result->fd = value;

    snprintf(buffer, sizeof(buffer), "/proc/%li/fd/%i", (long)iterator->pid, result->fd);
    if (betterReadlink(buffer, result->file, sizeof(result->file)) == -1) {
        int local_errno = errno;

        EMIT_ERROR("readlink failed on %s: %s", buffer, strerror(local_errno));
        return translateErrno(local_errno);
    }

    if (stat(buffer, &fs) != 0) {
        int local_errno = errno;

        EMIT_ERROR("stat failed on %s: %s", buffer, strerror(local_errno));
        return translateErrno(local_errno);
    }
    result->device = fs.st_dev;
    result->inode = fs.st_ino;

    return REAP_RET_OK;
}

int
reapReadFd(pid_t pid, int fd, char *dest, size_t size)
{
    char buffer[100];

    if (pid <= 0 || fd < 0 || !dest) {
        if (pid <= 0) {
            EMIT_ERROR("The PID must be positive");
        }
        else if (fd < 0) {
            EMIT_ERROR("The file descriptor cannot be negative");
        }
        else {
            EMIT_ERROR("The destination buffer cannot be NULL");
        }
        return REAP_RET_BAD_USAGE;
    }

    snprintf(buffer, sizeof(buffer), "/proc/%li/fd/%i", (long)pid, fd);
    if (betterReadlink(buffer, dest, size) == -1) {
        int local_errno = errno;

        EMIT_ERROR("readlink failed on %s: %s", buffer, strerror(local_errno));
        return translateErrno(local_errno);
    }

    return REAP_RET_OK;
}
