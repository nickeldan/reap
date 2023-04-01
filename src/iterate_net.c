#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <reap/iterate_net.h>

#include "internal.h"

struct reapNetIterator {
    FILE *file;
    unsigned int flags;
};

// Coped from uapi/linux/net.h in the Linux kernel headers.
typedef enum {
    SS_FREE = 0,     /* not allocated		*/
    SS_UNCONNECTED,  /* unconnected to any socket	*/
    SS_CONNECTING,   /* in process of connecting	*/
    SS_CONNECTED,    /* connected to socket		*/
    SS_DISCONNECTING /* in process of disconnecting	*/
} socket_state;

static char *
formFile(const reapNetIterator *iterator, char *dst, unsigned int size)
{
    if (iterator->flags & REAP_NET_FLAG_DOMAIN) {
        snprintf(dst, size, "/proc/net/unix");
    }
    else {
        snprintf(dst, size, "/proc/net/%sp%s", iterator->flags & REAP_NET_FLAG_UDP ? "ud" : "tc",
                 iterator->flags & REAP_NET_FLAG_IPV6 ? "6" : "");
    }
    return dst;
}

static char *
stripLine(char *line)
{
    unsigned int len;

    len = strlen(line);
    if (len > 0 && line[len - 1] == '\n') {
        line[len - 1] = '\0';
    }
    return line;
}

static int
nextUnix(const reapNetIterator *iterator, reapNetResult *result)
{
    char line[256];

    while (fgets(line, sizeof(line), iterator->file)) {
        unsigned int state;
        unsigned long inode_long;
        char path[256];

        path[0] = '\0';
        if (sscanf(line, "%*s %*s %*s %*s %i %u %lu %s", &result->socket_type, &state, &inode_long, path) <
            3) {
            emitError("Invalid line in /proc/net/unix: %s", stripLine(line));
            return REAP_RET_OTHER;
        }

        if (state > SS_DISCONNECTING) {
            emitError("Invalid socket state in /proc/net/unix: %u", state);
            return REAP_RET_OTHER;
        }

        result->connected = (state == SS_CONNECTED);
        if (path[0] == '\0' && !result->connected) {
            continue;
        }

        strncpy(result->path, path, sizeof(result->path));
        result->path[sizeof(result->path) - 1] = '\0';
        result->inode = inode_long;
        return REAP_RET_OK;
    }

    if (ferror(iterator->file)) {
        emitError("Failed to read from /proc/net/unix");
        return REAP_RET_FILE_READ;
    }

    return REAP_RET_DONE;
}

static int
parseNet4Line(const char *line, reapNetResult *result)
{
    unsigned int local_addr, remote_addr, local_port, remote_port;
    unsigned long inode_long;

    if (sscanf(line, " %*u: %8x:%x %8x:%x %*s %*s %*s %*s %*u %*u %lu", &local_addr, &local_port,
               &remote_addr, &remote_port, &inode_long) != 5) {
        return REAP_RET_OTHER;
    }

    memcpy(&result->local.address, &local_addr, 4);
    memcpy(&result->remote.address, &remote_addr, 4);

    result->local.port = local_port;
    result->remote.port = remote_port;
    result->inode = inode_long;

    return REAP_RET_OK;
}

static int
parseNet6Line(const char *line, reapNetResult *result)
{
    unsigned int local_port, remote_port;
    unsigned long inode_long;
    uint32_t local_addr[4], remote_addr[4];

    if (sscanf(line, " %*u: %8x%8x%8x%8x:%x %8x%8x%8x%8x:%x %*s %*s %*s %*s %*u %*u %lu", &local_addr[0],
               &local_addr[1], &local_addr[2], &local_addr[3], &local_port, &remote_addr[0], &remote_addr[1],
               &remote_addr[2], &remote_addr[3], &remote_port, &inode_long) != 11) {
        return REAP_RET_OTHER;
    }

    for (int k = 0; k < 4; k++) {
        memcpy(&result->local.address[4 * k], &local_addr[k], 4);
        memcpy(&result->remote.address[4 * k], &remote_addr[k], 4);
    }

    result->local.port = local_port;
    result->remote.port = remote_port;
    result->inode = inode_long;

    return REAP_RET_OK;
}

int
reapNetIteratorCreate(unsigned int flags, reapNetIterator **iterator)
{
    char buffer[20], line[256];

    if (!iterator) {
        emitError("The pointer cannot be NULL");
        return REAP_RET_BAD_USAGE;
    }

    *iterator = malloc(sizeof(**iterator));
    if (!*iterator) {
        emitError("Failed to allocate %zu bytes", sizeof(**iterator));
        return REAP_RET_OUT_OF_MEMORY;
    }

    (*iterator)->flags = flags;
    (*iterator)->file = fopen(formFile(*iterator, buffer, sizeof(buffer)), "r");
    if (!(*iterator)->file) {
        int local_errno = errno;

        emitError("Failed to open %s: %s", buffer, strerror(local_errno));
        free(*iterator);
        return -1 * local_errno;
    }

    if (!fgets(line, sizeof(line), (*iterator)->file)) {
        emitError("Failed to read from %s", buffer);
        reapNetIteratorDestroy(*iterator);
        return REAP_RET_FILE_READ;
    }

    return REAP_RET_OK;
}

void
reapNetIteratorDestroy(reapNetIterator *iterator)
{
    if (iterator) {
        fclose(iterator->file);
        free(iterator);
    }
}

int
reapNetIteratorNext(const reapNetIterator *iterator, reapNetResult *result)
{
    int ret;
    char line[256];

    if (!iterator || !iterator->file || !result) {
        if (!iterator) {
            emitError("The iterator cannot be NULL");
        }
        else {
            emitError("The result cannot be NULL");
        }
        return REAP_RET_BAD_USAGE;
    }

    result->flags = iterator->flags;

    if (iterator->flags & REAP_NET_FLAG_DOMAIN) {
        return nextUnix(iterator, result);
    }

    if (!fgets(line, sizeof(line), iterator->file)) {
        if (ferror(iterator->file)) {
            char buffer[20];

            emitError("Failed to read from %s", formFile(iterator, buffer, sizeof(buffer)));
            return REAP_RET_FILE_READ;
        }
        else {
            return REAP_RET_DONE;
        }
    }

    ret = (iterator->flags & REAP_NET_FLAG_IPV6 ? parseNet6Line : parseNet4Line)(line, result);
    if (ret == REAP_RET_OTHER) {
        char buffer[20];

        emitError("Malformed line in %s: %s", formFile(iterator, buffer, sizeof(buffer)), stripLine(line));
    }
    return ret;
}
