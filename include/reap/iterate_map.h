#ifndef REAP_ITERATE_MAP_H
#define REAP_ITERATE_MAP_H

#include <stdio.h>
#include <sys/mman.h>

#include "definitions.h"

typedef struct reapMapIterator {
    FILE *file;
} reapMapIterator;

typedef struct reapMapResult {
    unsigned long start;
    unsigned long end;
    unsigned int offset;
    int permissions;
    dev_t major_dev;
    dev_t minor_dev;
    ino_t inode;
    char file[REAP_SHORT_PATH_SIZE];
} reapMapResult;

int
reapMapIteratorInit(pid_t pid, reapMapIterator *iterator);

void
reapMapIteratorClose(reapMapIterator *iterator);

int
reapMapIteratorNext(const reapMapIterator *iterator, reapMapResult *result);

#endif  // REAP_ITERATE_MAP_H
