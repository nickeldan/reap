#include <sys/sysmacros.h>

#include <reap/iterate_map.h>

#include "internal.h"

int
reapMapIteratorInit(pid_t pid, reapMapIterator *iterator)
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

    snprintf(buffer, sizeof(buffer), "/proc/%li/maps", (long)pid);
    iterator->file = fopen(buffer, "r");
    if (!iterator->file) {
        int local_errno = errno;

        EMIT_ERROR("fopen failed to open %s: %s", buffer, strerror(local_errno));
        return -1 * local_errno;
    }

    return REAP_RET_OK;
}

void
reapMapIteratorClose(reapMapIterator *iterator)
{
    if (iterator && iterator->file) {
        fclose(iterator->file);
        iterator->file = NULL;
    }
}

int
reapMapIteratorNext(const reapMapIterator *iterator, reapMapResult *result)
{
    int num_matches;
    unsigned int major, minor;
    unsigned long inode;
    char r, w, x;
    char line[256];

    if (!iterator || !iterator->file || !result) {
        if (!iterator) {
            EMIT_ERROR("The iterator cannot be NULL");
        }
        else if (!iterator->file) {
            EMIT_ERROR("This iterator has been closed");
        }
        else {
            EMIT_ERROR("The result cannot be NULL");
        }
        return REAP_RET_BAD_USAGE;
    }

    if (!fgets(line, sizeof(line), iterator->file)) {
        if (ferror(iterator->file)) {
            EMIT_ERROR("Failed to read from maps file");
            return REAP_RET_FILE_READ;
        }
        else {
            return REAP_RET_DONE;
        }
    }
    num_matches = sscanf(line, "%lx-%lx %c%c%c%*c %x %x:%x %lu %s", &result->start, &result->end, &r, &w, &x,
                         &result->offset, &major, &minor, &inode, result->name);
    if (num_matches < 9) {
#ifndef REAP_NO_ERROR_BUFFER
        unsigned int line_length;

        line_length = strnlen(line, sizeof(line));
        if (line[line_length - 1] == '\n') {
            line[line_length - 1] = '\0';
        }
        EMIT_ERROR("Malformed line in maps file: %s", line);
#endif
        return REAP_RET_OTHER;
    }

    if (num_matches == 9) {
        result->name[0] = '\0';
    }

    result->permissions = 0;
    if (r == 'r') {
        result->permissions |= PROT_READ;
    }
    if (w == 'w') {
        result->permissions |= PROT_WRITE;
    }
    if (x == 'x') {
        result->permissions |= PROT_EXEC;
    }

    result->device = makedev(major, minor);
    result->inode = inode;

    return REAP_RET_OK;
}
