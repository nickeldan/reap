#include <errno.h>
#include <stdlib.h>

#include "reap/iterate_proc.h"

#include "internal.h"

int
reapProcIteratorInit(reapProcIterator *iterator)
{
    if (!iterator) {
        return REAP_RET_BAD_USAGE;
    }

    iterator->dir = opendir("/proc");
    return iterator->dir ? REAP_RET_OK : translateErrno();
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
            return (errno == 0) ? REAP_RET_DONE : translateErrno();
        }

        value = strtol(entry->d_name, &endptr, 10);
        if (*endptr == '\0' && value > 0 && (pid = value) == value) {
            if (reapGetProcInfo(pid, info) == REAP_RET_OK) {
                return REAP_RET_OK;
            }
        }
    }
}
