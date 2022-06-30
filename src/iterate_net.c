#include <arpa/inet.h>

#include <reap/iterate_net.h>

#include "internal.h"

static char *
formFile(const reapNetIterator *iterator, char *dst, unsigned int size)
{
    snprintf(dst, size, "/proc/net/%sp%s", iterator->udp ? "ud" : "tc", iterator->ipv6 ? "6" : "");
    return dst;
}

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

    iterator->udp = !!(flags & REAP_NET_FLAG_UDP);
    iterator->ipv6 = !!(flags & REAP_NET_FLAG_IPV6);

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

    if (!fgets(line, sizeof(line), iterator->file)) {
        if (ferror(iterator->file)) {
#ifdef REAP_USE_ERROR_BUFFER
            char buffer[20];

            EMIT_ERROR("Failed to read from %s", formFile(iterator, buffer, sizeof(buffer)));
#endif
            return REAP_RET_FILE_READ;
        }
        else {
            return REAP_RET_DONE;
        }
    }

    result->udp = iterator->udp;
    result->ipv6 = iterator->ipv6;

    ret = (iterator->ipv6 ? parseNet6Line : parseNet4Line)(line, result);
#ifdef REAP_USE_ERROR_BUFFER
    if (ret == REAP_RET_OTHER) {
        unsigned int len;
        char buffer[20];

        len = strnlen(line, sizeof(line));
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        EMIT_ERROR("Malformed line in %s: %s", formFile(iterator, buffer, sizeof(buffer)), line);
    }
#endif
    return ret;
}
