#include <arpa/inet.h>

#include <reap/iterate_net.h>

#include "internal.h"

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

#ifndef REAP_NO_ERROR_BUFFER

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

#endif

static void
storeU32(uint32_t num, uint8_t *dst)
{
    num = htonl(num);

    for (int k = 3; k >= 0; k--) {
        dst[k] = num & 0xff;
        num >>= 8;
    }
}

static unsigned char
hexToNum(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    else {
        return c - 'a' + 10;
    }
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
            EMIT_ERROR("Invalid line in /proc/net/unix: %s", stripLine(line));
            return REAP_RET_OTHER;
        }

        if (state > SS_DISCONNECTING) {
            EMIT_ERROR("Invalid socket state in /proc/net/unix: %u", state);
            return REAP_RET_OTHER;
        }

        result->connected = (state == SS_CONNECTED);
        if (path[0] == '\0' && !result->connected) {
            continue;
        }
        if (snprintf(result->path, sizeof(result->path), "%s", path) < 0) {
            (void)0;  // Suppresses -Wformat-truncation warning.
        }
        result->inode = inode_long;
        return REAP_RET_OK;
    }

    if (ferror(iterator->file)) {
        EMIT_ERROR("Failed to read from /proc/net/unix");
        return REAP_RET_FILE_READ;
    }

    return REAP_RET_DONE;
}

static void
populateAddr6(uint8_t *addr, const char *hex)
{
    for (int k = 0; k < IPV6_SIZE; k++) {
        addr[k] = (hexToNum(hex[2 * k]) << 4) | hexToNum(hex[2 * k + 1]);
    }
}

static int
parseNet4Line(const char *line, reapNetResult *result)
{
    unsigned int local_addr, remote_addr, local_port, remote_port;
    unsigned long inode_long;

    if (sscanf(line, " %*u: %x:%x %x:%x %*s %*s %*s %*s %*u %*u %lu", &local_addr, &local_port, &remote_addr,
               &remote_port, &inode_long) != 5) {
        return REAP_RET_OTHER;
    }

    storeU32(local_addr, result->local.address);
    result->local.port = local_port;

    storeU32(remote_addr, result->remote.address);
    result->remote.port = remote_port;

    result->inode = inode_long;

    return REAP_RET_OK;
}

static int
parseNet6Line(const char *line, reapNetResult *result)
{
    unsigned int local_port, remote_port;
    unsigned long inode_long;
    char local_addr[33], remote_addr[33];

    if (sscanf(line, " %*u: %32s:%x %32s:%x %*s %*s %*s %*s %*u %*u %lu", local_addr, &local_port,
               remote_addr, &remote_port, &inode_long) != 5) {
        return REAP_RET_OTHER;
    }

    populateAddr6(result->local.address, local_addr);
    result->local.port = local_port;

    populateAddr6(result->remote.address, remote_addr);
    result->remote.port = remote_port;

    result->inode = inode_long;

    return REAP_RET_OK;
}

int
reapNetIteratorInit(reapNetIterator *iterator, unsigned int flags)
{
    char buffer[20], line[256];

    if (!iterator) {
        EMIT_ERROR("The iterator cannot be NULL");
        return REAP_RET_BAD_USAGE;
    }

    iterator->flags = flags;
    iterator->file = fopen(formFile(iterator, buffer, sizeof(buffer)), "r");
    if (!iterator->file) {
        int local_errno = errno;

        EMIT_ERROR("Failed to open %s: %s", buffer, strerror(local_errno));
        return translateErrno(local_errno);
    }

    if (!fgets(line, sizeof(line), iterator->file)) {
        EMIT_ERROR("Failed to read from %s", buffer);
        reapNetIteratorClose(iterator);
        return REAP_RET_FILE_READ;
    }

    return REAP_RET_OK;
}

void
reapNetIteratorClose(reapNetIterator *iterator)
{
    if (iterator && iterator->file) {
        fclose(iterator->file);
        iterator->file = NULL;
    }
}

int
reapNetIteratorNext(const reapNetIterator *iterator, reapNetResult *result)
{
    int ret;
    char line[256];

    if (!iterator || !iterator->file || !result) {
        if (!iterator) {
            EMIT_ERROR("The iterator cannot be NULL");
        }
        else if (!iterator->file) {
            EMIT_ERROR("This iterator has been closed");
        }
        else {
            EMIT_ERROR("The result cannot be NULL");
        }
        return REAP_RET_BAD_USAGE;
    }

    result->flags = iterator->flags;

    if (iterator->flags & REAP_NET_FLAG_DOMAIN) {
        return nextUnix(iterator, result);
    }

    if (!fgets(line, sizeof(line), iterator->file)) {
        if (ferror(iterator->file)) {
#ifndef REAP_NO_ERROR_BUFFER
            char buffer[20];

            EMIT_ERROR("Failed to read from %s", formFile(iterator, buffer, sizeof(buffer)));
#endif
            return REAP_RET_FILE_READ;
        }
        else {
            return REAP_RET_DONE;
        }
    }

    ret = (iterator->flags & REAP_NET_FLAG_IPV6 ? parseNet6Line : parseNet4Line)(line, result);
    if (ret == REAP_RET_OTHER) {
#ifndef REAP_NO_ERROR_BUFFER
        char buffer[20];

        EMIT_ERROR("Malformed line in %s: %s", formFile(iterator, buffer, sizeof(buffer)), stripLine(line));
#endif
    }
    return ret;
}
