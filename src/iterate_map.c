#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sysmacros.h>

#include <reap/iterate_map.h>

#include "internal.h"

struct reapMapIterator {
    FILE *file;
#ifndef REAP_NO_ERROR_BUFFER
    pid_t pid;
#endif
};

static char *
formPath(pid_t pid, char *dst, size_t size)
{
    snprintf(dst, size, "/proc/%li/maps", (long)pid);
    return dst;
}

int
reapMapIteratorCreate(pid_t pid, reapMapIterator **iterator)
{
    char buffer[30];

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

#ifndef REAP_NO_ERROR_BUFFER
    (*iterator)->pid = pid;
#endif
    (*iterator)->file = fopen(formPath(pid, buffer, sizeof(buffer)), "r");
    if (!(*iterator)->file) {
        int local_errno = errno;

        EMIT_ERROR("fopen failed to open %s: %s", buffer, strerror(local_errno));
        free(*iterator);
        return -1 * local_errno;
    }

    return REAP_RET_OK;
}

void
reapMapIteratorDestroy(reapMapIterator *iterator)
{
    if (iterator) {
        fclose(iterator->file);
        free(iterator);
    }
}

int
reapMapIteratorNext(const reapMapIterator *iterator, reapMapResult *result, char *name, size_t name_size)
{
    int num_matches;
    unsigned int major, minor;
    unsigned long inode;
    char r, w, x;
    char line[PATH_MAX + 100], buffer[PATH_MAX];

    if (!iterator || !result) {
        if (!iterator) {
            EMIT_ERROR("The iterator cannot be NULL");
        }
        else {
            EMIT_ERROR("The result cannot be NULL");
        }
        return REAP_RET_BAD_USAGE;
    }

    if (!fgets(line, sizeof(line), iterator->file)) {
        if (ferror(iterator->file)) {
#ifndef REAP_NO_ERROR_BUFFER
            char error_buffer[30];

            EMIT_ERROR("Failed to read from %s",
                       formPath(iterator->pid, error_buffer, sizeof(error_buffer)));
#endif
            return REAP_RET_FILE_READ;
        }
        else {
            return REAP_RET_DONE;
        }
    }
    num_matches = sscanf(line, "%llx-%llx %c%c%c%*c %llx %x:%x %lu %s", &result->start, &result->end, &r, &w,
                         &x, &result->offset, &major, &minor, &inode, buffer);
    if (num_matches < 9) {
#ifndef REAP_NO_ERROR_BUFFER
        unsigned int line_length;
        char error_buffer[30];

        line_length = strnlen(line, sizeof(line));
        if (line[line_length - 1] == '\n') {
            line[line_length - 1] = '\0';
        }
        EMIT_ERROR("Malformed line in %s: %s", formPath(iterator->pid, error_buffer, sizeof(error_buffer)),
                   line);
#endif
        return REAP_RET_OTHER;
    }

    if (num_matches == 9) {
        buffer[0] = '\0';
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

    if (name) {
        snprintf(name, name_size, "%s", buffer);
    }

    return REAP_RET_OK;
}
