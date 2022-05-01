#include <stdio.h>

#include <reap/reap.h>

int
main()
{
    int ret;
    reapProcIterator iterator;
    reapProcInfo info;

    ret = reapProcIteratorInit(&iterator);
    if (ret != REAP_RET_OK) {
#ifdef REAP_USE_ERROR_BUFFER
        fprintf(stderr, "reapProcIteratorInit: %s\n", reapGetError());
#else
        fprintf(stderr, "reapProcIteratorInit: %s\n", reapErrorString(ret));
#endif
        return ret;
    }

    while ((ret = reapProcIteratorNext(&iterator, &info)) == REAP_RET_OK) {
        printf("%li: %s\n", (long)info.pid, info.exe);
    }

    reapProcIteratorClose(&iterator);

    if (ret == REAP_RET_DONE) {
        ret = REAP_RET_OK;
    }
    else {
#ifdef REAP_USE_ERROR_BUFFER
        fprintf(stderr, "reapProcIteratorNext: %s\n", reapGetError());
#else
        fprintf(stderr, "reapProcIteratorNext: %s\n", reapErrorString(ret));
#endif
    }

    return ret;
}
