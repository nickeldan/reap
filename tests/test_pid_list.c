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
        fprintf(stderr, "reapProcIteratorInit: %s", reapErrorString(ret));
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
        fprintf(stderr, "reapProcIteratorNext: %s", reapErrorString(ret));
    }

    return ret;
}
