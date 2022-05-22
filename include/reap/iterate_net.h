#ifndef REAP_ITERATE_NET_H
#define REAP_ITERATE_NET_H

#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>

#include "definitions.h"

typedef struct reapNetIterator {
    FILE *file;
    unsigned int tcp : 1;
} reapNetIterator;

typedef struct reapNetResult {
    struct sockaddr_in local;
    struct sockaddr_in remote;
    ino_t inode;
} reapNetResult;

typedef struct reapNet6Iterator {
    FILE *file;
    unsigned int tcp : 1;
} reapNet6Iterator;

typedef struct reapNet6Result {
    struct sockaddr_in6 local;
    struct sockaddr_in6 remote;
    ino_t inode;
} reapNet6Result;

int
reapNetIteratorInit(reapNetIterator *iterator, bool tcp);

void
reapNetIteratorClose(reapNetIterator *iterator);

int
reapNetIteratorNext(const reapNetIterator *iterator, reapNetResult *result);

int
reapNet6IteratorInit(reapNet6Iterator *iterator, bool tcp);

void
reapNet6IteratorClose(reapNet6Iterator *iterator);

int
reapNet6IteratorNext(const reapNet6Iterator *iterator, reapNet6Result *result);

#endif  // REAP_ITERATE_NET_H
