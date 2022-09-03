#include <stdio.h>
#include <stdlib.h>

#include "common.h"

int
main(int argc, char **argv)
{
    int ret;
    long value;
    pid_t pid, thread;
    char *endptr;
    reapThreadIterator *iterator;

    if (argc < 2) {
        fprintf(stderr, "Missing argument\n");
        return REAP_RET_BAD_USAGE;
    }

    value = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0' || value <= 0 || (pid = value) != value) {
        fprintf(stderr, "Invalid PID\n");
        return REAP_RET_BAD_USAGE;
    }

    ret = reapThreadIteratorCreate(pid, &iterator);
    if (ret != REAP_RET_OK) {
        fprintf(stderr, "reapThreadIteratorCreate: %s", ERROR(ret));
        return ret;
    }

    while ((ret = reapThreadIteratorNext(iterator, &thread)) == REAP_RET_OK) {
        printf("%li\n", (long)thread);
    }

    reapThreadIteratorDestroy(iterator);

    if (ret == REAP_RET_DONE) {
        ret = REAP_RET_OK;
    }
    else {
        fprintf(stderr, "reapThreadIteratorNext: %s\n", ERROR(ret));
    }

    return ret;
}
