#include <stdio.h>

#include "common.h"

int
main()
{
    int ret;
    reapProcIterator iterator;
    reapProcInfo info;

    ret = reapProcIteratorInit(&iterator);
    if (ret != REAP_RET_OK) {
        fprintf(stderr, "reapProcIteratorInit: %s\n", ERROR(ret));
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
        fprintf(stderr, "reapProcIteratorNext: %s\n", ERROR(ret));
    }

    return ret;
}
