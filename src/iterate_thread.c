#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <reap/iterate_thread.h>

#include "internal.h"

static char *
formDirectory(pid_t pid, char *dst, unsigned int size)
{
    snprintf(dst, size, "/proc/%li/task", (long)pid);
    return dst;
}

int
reapThreadIteratorInit(pid_t pid, reapThreadIterator *iterator)
{
    char buffer[30];

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
    iterator->dir = opendir(formDirectory(pid, buffer, sizeof(buffer)));
    if (!iterator->dir) {
        int local_errno = errno;

        EMIT_ERROR("opendir failed on %s: %s", buffer, strerror(local_errno));
        return translateErrno(local_errno);
    }

    return REAP_RET_OK;
}

void
reapThreadIteratorClose(reapThreadIterator *iterator)
{
    if (iterator && iterator->dir) {
        closedir(iterator->dir);
        iterator->dir = NULL;
    }
}

int
reapThreadIteratorNext(const reapThreadIterator *iterator, pid_t *thread)
{
    long value;
    char *endptr;
    struct dirent *entry;

    if (!iterator || !iterator->dir || !thread) {
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
    if (*endptr != '\0' || value <= 0 || (*thread = value) != value) {
#ifdef REAP_USE_ERROR_BUFFER
        char buffer[30];

        EMIT_ERROR("Invalid file in %s: %s", formDirectory(iterator->pid, buffer, sizeof(buffer)),
                   entry->d_name);
#endif
        return REAP_RET_OTHER;
    }

    return REAP_RET_OK;
}
