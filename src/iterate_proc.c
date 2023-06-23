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
        reapEmitError("The pointer cannot be NULL");
        return REAP_RET_BAD_USAGE;
    }

    *iterator = malloc(sizeof(**iterator));
    if (!*iterator) {
        reapEmitError("Failed to allocate %zu bytes", sizeof(**iterator));
        return REAP_RET_OUT_OF_MEMORY;
    }

    (*iterator)->dir = opendir("/proc");
    if (!(*iterator)->dir) {
        int local_errno = errno;

        reapEmitError("opendir failed on /proc: %s", strerror(local_errno));
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
reapProcIteratorNext(const reapProcIterator *iterator, reapProcInfo *info, char *exe_path, size_t path_size)
{
    if (!iterator || !iterator->dir || !info) {
        if (!iterator) {
            reapEmitError("The iterator cannot be NULL");
        }
        else {
            reapEmitError("The info cannot be NULL");
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
                reapEmitError("readdir failed: %s", strerror(local_errno));
                return -1 * local_errno;
            }
        }

        value = strtol(entry->d_name, &endptr, 10);
        if (*endptr == '\0' && value > 0 && (pid = value) == value) {
            if (reapGetProcInfo(pid, info, exe_path, path_size) == REAP_RET_OK) {
                return REAP_RET_OK;
            }
        }
    }
}
