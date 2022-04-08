#ifndef REAP_ITERATE_PROC_H
#define REAP_ITERATE_PROC_H

#include <dirent.h>

#include "definitions.h"

typedef struct reapProcIterator {
    DIR *dir;
} reapProcIterator;

int
reapProcIteratorInit(reapProcIterator *iterator);

void
reapProcIteratorClose(reapProcIterator *iterator);

int
reapProcIteratorNext(reapProcIterator *iterator, reapProcInfo *info);

#endif // REAP_ITERATE_PROC_H
