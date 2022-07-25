#include <stdio.h>
#include <stdlib.h>

#include "common.h"

int
main(int argc, char **argv)
{
    int ret;
    long value;
    pid_t pid;
    char *endptr;
    reapMapIterator iterator;
    reapMapResult result;

    if (argc < 2) {
        fprintf(stderr, "Missing argument\n");
        return REAP_RET_BAD_USAGE;
    }

    value = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0' || value <= 0 || (pid = value) != value) {
        fprintf(stderr, "Invalid PID\n");
        return REAP_RET_BAD_USAGE;
    }

    ret = reapMapIteratorInit(pid, &iterator);
    if (ret != REAP_RET_OK) {
        fprintf(stderr, "reapMapIteratorInit: %s\n", ERROR(ret));
        return ret;
    }

    while ((ret = reapMapIteratorNext(&iterator, &result)) == REAP_RET_OK) {
        printf("%lx - %lx ", result.start, result.end);
        printf("%c", (result.permissions & PROT_READ) ? 'r' : '-');
        printf("%c", (result.permissions & PROT_WRITE) ? 'w' : '-');
        printf("%c", (result.permissions & PROT_EXEC) ? 'x' : '-');
        printf(" %08x %u %lu %s\n", result.offset, (unsigned int)result.device, (unsigned long)result.inode,
               result.name);
    }

    reapMapIteratorClose(&iterator);

    if (ret == REAP_RET_DONE) {
        ret = REAP_RET_OK;
    }
    else {
        fprintf(stderr, "reapMapIteratorNext: %s\n", ERROR(ret));
    }

    return ret;
}
