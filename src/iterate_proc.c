#include <stdlib.h>

#include "reap/iterate_proc.h"

#include "internal.h"

int
reapProcIteratorInit(reapProcIterator *iterator)
{
    if (!iterator) {
        EMIT_ERROR("The iterator cannot be NULL");
        return REAP_RET_BAD_USAGE;
    }

    iterator->dir = opendir("/proc");
    if (!iterator->dir) {
        int local_errno = errno;

        EMIT_ERROR("opendir failed on /proc: %s", strerror(local_errno));
        return translateErrno(local_errno);
    }

    return REAP_RET_OK;
}

void
reapProcIteratorClose(reapProcIterator *iterator)
{
    if (iterator && iterator->dir) {
        closedir(iterator->dir);
        iterator->dir = NULL;
    }
}

int
reapProcIteratorNext(const reapProcIterator *iterator, reapProcInfo *info)
{
    if (!iterator || !iterator->dir || !info) {
        if (!iterator) {
            EMIT_ERROR("The iterator cannot be NULL");
        }
        else if (!iterator->dir) {
            EMIT_ERROR("This iterator has been closed");
        }
        else {
            EMIT_ERROR("The info cannot be NULL");
        }
        return REAP_RET_BAD_USAGE;
    }

    while (1) {
        long value;
        pid_t pid;
        char *endptr;
        struct dirent *entry;

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

        value = strtol(entry->d_name, &endptr, 10);
        if (*endptr == '\0' && value > 0 && (pid = value) == value) {
            if (reapGetProcInfo(pid, info) == REAP_RET_OK) {
                return REAP_RET_OK;
            }
        }
    }
}
