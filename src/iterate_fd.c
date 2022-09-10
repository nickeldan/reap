#include <dirent.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <reap/iterate_fd.h>

#include "internal.h"

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
            EMIT_ERROR("The PID must be positive");
        }
        else {
            EMIT_ERROR("The pointer cannot be NULL");
        }
        return REAP_RET_BAD_USAGE;
    }

    *iterator = malloc(sizeof(**iterator));
    if (!*iterator) {
        EMIT_ERROR("Failed to allocate %zu bytes", sizeof(**iterator));
        return REAP_RET_OUT_OF_MEMORY;
    }

    (*iterator)->pid = pid;
    (*iterator)->dir = opendir(formPath(pid, buffer, sizeof(buffer)));
    if (!(*iterator)->dir) {
        int local_errno = errno;

        EMIT_ERROR("opendir failed on %s: %s", buffer, strerror(local_errno));
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
    char base_buffer[30], buffer[40], path[PATH_MAX];
    struct dirent *entry;
    struct stat fs;

    if (!iterator || !result) {
        if (!iterator) {
            EMIT_ERROR("The iterator cannot be NULL");
        }
        else {
            EMIT_ERROR("The result cannot be NULL");
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
                    EMIT_ERROR("readdir failed: %s", strerror(local_errno));
                    return -1 * local_errno;
                }
            }
        } while (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0);

        value = strtol(entry->d_name, &endptr, 10);
        if (*endptr != '\0' || value < 0 || (result->fd = value) != value) {
            EMIT_ERROR("Invalid file in %s: %s", base_buffer, entry->d_name);
            return REAP_RET_OTHER;
        }

        snprintf(buffer, sizeof(buffer), "%s/%i", base_buffer, result->fd);
    } while (betterReadlink(buffer, path, sizeof(path)) == -1 || stat(buffer, &fs) != 0);

    result->device = fs.st_dev;
    result->inode = fs.st_ino;
    result->mode = fs.st_mode;

    if (file) {
        snprintf(file, file_size, "%s", path);
    }

    return REAP_RET_OK;
}
