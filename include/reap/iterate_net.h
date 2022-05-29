#ifndef REAP_ITERATE_NET_H
#define REAP_ITERATE_NET_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

#include "definitions.h"

#ifndef IPV4_SIZE
#define IPV4_SIZE 4
#endif
#ifndef IPV6_SIZE
#define IPV6_SIZE 16
#endif

typedef struct reapNetIterator {
    FILE *file;
    unsigned int tcp : 1;
} reapNetIterator;

typedef struct reapNetPeer {
    uint16_t port;
    uint8_t address[IPV4_SIZE];
} reapNetPeer;

typedef struct reapNetResult {
    reapNetPeer local;
    reapNetPeer remote;
    ino_t inode;
} reapNetResult;

typedef struct reapNet6Iterator {
    FILE *file;
    unsigned int tcp : 1;
} reapNet6Iterator;

typedef struct reapNet6Peer {
    uint16_t port;
    uint8_t address[IPV6_SIZE];
} reapNet6Peer;

typedef struct reapNet6Result {
    reapNet6Peer local;
    reapNet6Peer remote;
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
