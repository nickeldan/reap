#ifndef REAP_ITERATE_MAP_H
#define REAP_ITERATE_MAP_H

#include <stdint.h>
#include <stdio.h>

#include "definitions.h"

typedef struct reapMapIterator {
    FILE *file;
} reapMapIterator;

typedef struct reapMapResult {
    uintptr_t start;
    uintptr_t end;
    int permissions;
    char file[SHORT_PATH_SIZE];
} reapMapResult;

int
reapMapIteratorInit(pid_t pid, reapMapIterator *iterator);

void
reapMapIteratorClose(reapMapIterator *iterator);

int
reapMapIteratorNext(const reapMapIterator *iterator, reapMapResult *result);

#endif  // REAP_ITERATE_MAP_H
