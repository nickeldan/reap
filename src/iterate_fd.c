#include <dirent.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <reap/iterate_fd.h>

#include "internal.h"

#ifndef REAP_NO_ITERATE_FD

struct reapFdIterator {
    DIR *dir;
    pid_t pid;
};

static char *
formPath(pid_t pid, char *dst, size_t size)
{
    snprintf(dst, size, "/proc/%li/fd", (long)pid);
    return dst;
}

int
reapFdIteratorCreate(pid_t pid, reapFdIterator **iterator)
{
    char buffer[40];

    if (pid <= 0 || !iterator) {
        if (pid <= 0) {
            reapEmitError("The PID must be positive");
        }
        else {
            reapEmitError("The pointer cannot be NULL");
        }
        return REAP_RET_BAD_USAGE;
    }

    *iterator = malloc(sizeof(**iterator));
    if (!*iterator) {
        reapEmitError("Failed to allocate %zu bytes", sizeof(**iterator));
        return REAP_RET_OUT_OF_MEMORY;
    }

    (*iterator)->pid = pid;
    (*iterator)->dir = opendir(formPath(pid, buffer, sizeof(buffer)));
    if (!(*iterator)->dir) {
        int local_errno = errno;

        reapEmitError("opendir failed on %s: %s", buffer, strerror(local_errno));
        free(*iterator);
        return -1 * local_errno;
    }

    return REAP_RET_OK;
}

void
reapFdIteratorDestroy(reapFdIterator *iterator)
{
    if (iterator) {
        closedir(iterator->dir);
        free(iterator);
    }
}

int
reapFdIteratorNext(reapFdIterator *iterator, reapFdResult *result, char *file, size_t file_size)
{
    char base_buffer[30], buffer[50], path[PATH_MAX];
    struct dirent *entry;
    struct stat fs;

    if (!iterator || !result) {
        if (!iterator) {
            reapEmitError("The iterator cannot be NULL");
        }
        else {
            reapEmitError("The result cannot be NULL");
        }
        return REAP_RET_BAD_USAGE;
    }

    formPath(iterator->pid, base_buffer, sizeof(base_buffer));

    do {
        long value;
        char *endptr;

        do {
            errno = 0;
            entry = readdir(iterator->dir);
            if (!entry) {
                int local_errno = errno;

                if (local_errno == 0) {
                    return REAP_RET_DONE;
                }
                else {
                    reapEmitError("readdir failed: %s", strerror(local_errno));
                    return -1 * local_errno;
                }
            }
        } while (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0);

        value = strtol(entry->d_name, &endptr, 10);
        if (*endptr != '\0' || value < 0 || (result->fd = value) != value) {
            reapEmitError("Invalid file in %s: %s", base_buffer, entry->d_name);
            return REAP_RET_OTHER;
        }

        snprintf(buffer, sizeof(buffer), "%s/%i", base_buffer, result->fd);
    } while (reapBetterReadlink(buffer, path, sizeof(path)) == -1 || stat(buffer, &fs) != 0);

    result->device = fs.st_dev;
    result->inode = fs.st_ino;
    result->mode = fs.st_mode;

    if (file) {
        snprintf(file, file_size, "%s", path);
    }

    return REAP_RET_OK;
}

#endif  // REAP_NO_ITERATE_FD
