#include <stdio.h>
#include <stdlib.h>

#include <reap/iterate_map.h>

int
main(int argc, char **argv)
{
    int ret;
    long value;
    pid_t pid;
    char *endptr;
    reapMapIterator iterator;
    reapMapResult result;

    if (argc == 1) {
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
#ifdef REAP_USE_ERROR_BUFFER
        fprintf(stderr, "reapMapIteratorInit: %s\n", reapGetError());
#else
        fprintf(stderr, "reapMapIteratorInit: %s\n", reapErrorString(ret));
#endif
        return ret;
    }

    while ((ret = reapMapIteratorNext(&iterator, &result)) == REAP_RET_OK) {
        printf("%lx - %lx ", result.start, result.end);
        printf("%c", (result.permissions & PROT_READ) ? 'r' : '-');
        printf("%c", (result.permissions & PROT_WRITE) ? 'w' : '-');
        printf("%c", (result.permissions & PROT_EXEC) ? 'x' : '-');
        printf(" %08x %u %lu %s\n", result.offset, (unsigned int)result.device, (unsigned long)result.inode,
               result.file);
    }

    if (ret == REAP_RET_DONE) {
        ret = REAP_RET_OK;
    }
    else {
#ifdef REAP_USE_ERROR_BUFFER
        fprintf(stderr, "reapMapIteratorNext: %s\n", reapGetError());
#else
        fprintf(stderr, "reapMapIteratorNext: %s\n", reapErrorString(ret));
#endif
    }

    reapMapIteratorClose(&iterator);
    return ret;
}
