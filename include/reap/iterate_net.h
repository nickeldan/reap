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
    unsigned int udp : 1;
    unsigned int ipv6 : 1;
} reapNetIterator;

typedef struct reapNetPeer {
    uint16_t port;
    uint8_t address[IPV6_SIZE];
} reapNetPeer;

typedef struct reapNetResult {
    reapNetPeer local;
    reapNetPeer remote;
    ino_t inode;
    unsigned int udp : 1;
    unsigned int ipv6 : 1;
} reapNetResult;

#define REAP_NET_FLAG_UDP  0x01
#define REAP_NET_FLAG_IPV6 0x02

int
reapNetIteratorInit(reapNetIterator *iterator, unsigned int flags);

void
reapNetIteratorClose(reapNetIterator *iterator);

int
reapNetIteratorNext(const reapNetIterator *iterator, reapNetResult *result);

#endif  // REAP_ITERATE_NET_H
