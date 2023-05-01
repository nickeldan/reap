#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <reap/iterate_thread.h>

#include "internal.h"

#ifndef REAP_NO_ITERATE_THREAD

struct reapThreadIterator {
    DIR *dir;
    pid_t pid;
};

static char *
formDirectory(pid_t pid, char *dst, unsigned int size)
{
    snprintf(dst, size, "/proc/%li/task", (long)pid);
    return dst;
}

int
reapThreadIteratorCreate(pid_t pid, reapThreadIterator **iterator)
{
    char buffer[30];

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
    (*iterator)->dir = opendir(formDirectory(pid, buffer, sizeof(buffer)));
    if (!(*iterator)->dir) {
        int local_errno = errno;

        reapEmitError("opendir failed on %s: %s", buffer, strerror(local_errno));
        free(*iterator);
        return -1 * local_errno;
    }

    return REAP_RET_OK;
}

void
reapThreadIteratorDestroy(reapThreadIterator *iterator)
{
    if (iterator) {
        closedir(iterator->dir);
        free(iterator);
    }
}

int
reapThreadIteratorNext(const reapThreadIterator *iterator, pid_t *thread)
{
    long value;
    char *endptr;
    struct dirent *entry;

    if (!iterator || !thread) {
        if (!iterator) {
            reapEmitError("The iterator cannot be NULL");
        }
        else {
            reapEmitError("The result cannot be NULL");
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
                reapEmitError("readdir failed: %s", strerror(local_errno));
                return -1 * local_errno;
            }
        }
    } while (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0);

    value = strtol(entry->d_name, &endptr, 10);
    if (*endptr != '\0' || value <= 0 || (*thread = value) != value) {
        char buffer[30];

        reapEmitError("Invalid file in %s: %s", formDirectory(iterator->pid, buffer, sizeof(buffer)),
                      entry->d_name);
        return REAP_RET_OTHER;
    }

    return REAP_RET_OK;
}

#endif  // REAP_NO_ITERATE_THREAD
