#include <stdio.h>
#include <stdlib.h>

#include <reap/reap.h>

int
main(int argc, char **argv)
{
    int ret;
    long value;
    pid_t pid;
    char *endptr;
    reapFdIterator iterator;
    reapFdResult result;

    if (argc == 1) {
        fprintf(stderr, "Missing argument\n");
        return REAP_RET_BAD_USAGE;
    }

    value = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0' || value <= 0 || (pid = value) != value) {
        fprintf(stderr, "Invalid PID\n");
        return REAP_RET_BAD_USAGE;
    }

    ret = reapFdIteratorInit(pid, &iterator);
    if (ret != REAP_RET_OK) {
        fprintf(stderr, "reapFdIteratorInit: %s\n", reapErrorString(ret));
        return ret;
    }

    while ((ret = reapFdIteratorNext(&iterator, &result)) == REAP_RET_OK) {
        printf("%i: %s\n", result.fd, result.file);
    }

    if (ret == REAP_RET_DONE) {
        ret = REAP_RET_OK;
    }
    else {
#ifdef REAP_USE_ERROR_BUFFER
        fprintf(stderr, "reapFdIteratorNext: %s\n", reapGetError());
#else
        fprintf(stderr, "reapFdIteratorNext: %s\n", reapErrorString(ret));
#endif
    }

    reapFdIteratorClose(&iterator);
    return ret;
}
