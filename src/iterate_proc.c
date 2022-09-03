#include <dirent.h>
#include <stdlib.h>

#include "reap/iterate_proc.h"

#include "internal.h"

struct reapProcIterator {
    DIR *dir;
};

int
reapProcIteratorCreate(reapProcIterator **iterator)
{
    if (!iterator) {
        EMIT_ERROR("The pointer cannot be NULL");
        return REAP_RET_BAD_USAGE;
    }

    *iterator = malloc(sizeof(**iterator));
    if (!*iterator) {
        EMIT_ERROR("Failed to allocate %zu bytes", sizeof(**iterator));
        return REAP_RET_OUT_OF_MEMORY;
    }

    (*iterator)->dir = opendir("/proc");
    if (!(*iterator)->dir) {
        int local_errno = errno;

        EMIT_ERROR("opendir failed on /proc: %s", strerror(local_errno));
        free(*iterator);
        return -1 * local_errno;
    }

    return REAP_RET_OK;
}

void
reapProcIteratorDestroy(reapProcIterator *iterator)
{
    if (iterator) {
        closedir(iterator->dir);
        free(iterator);
    }
}

int
reapProcIteratorNext(const reapProcIterator *iterator, reapProcInfo *info)
{
    if (!iterator || !iterator->dir || !info) {
        if (!iterator) {
            EMIT_ERROR("The iterator cannot be NULL");
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
                return -1 * local_errno;
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
