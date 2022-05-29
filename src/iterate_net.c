#include <arpa/inet.h>

#include <reap/iterate_net.h>

#include "internal.h"

static void
iteratorClose(FILE **f)
{
    if (*f) {
        fclose(*f);
        *f = NULL;
    }
}

static int
iteratorInit(FILE **f, const char *file)
{
    char line[256];

    *f = fopen(file, "r");
    if (!*f) {
        int local_errno = errno;

        EMIT_ERROR("Failed to open %s: %s", file, strerror(local_errno));
        return translateErrno(local_errno);
    }

    if (!fgets(line, sizeof(line), *f)) {
        EMIT_ERROR("Failed to read from %s", file);
        iteratorClose(f);
        return REAP_RET_OTHER;
    }

    return REAP_RET_OK;
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

int
reapNetIteratorInit(reapNetIterator *iterator, bool tcp)
{
    char buffer[20];

    if (!iterator) {
        EMIT_ERROR("The iterator cannot be NULL");
        return REAP_RET_BAD_USAGE;
    }

    iterator->tcp = tcp;
    snprintf(buffer, sizeof(buffer), "/proc/net/%sp", tcp ? "tc" : "ud");
    return iteratorInit(&iterator->file, buffer);
}

void
reapNetIteratorClose(reapNetIterator *iterator)
{
    if (iterator) {
        iteratorClose(&iterator->file);
    }
}

int
reapNetIteratorNext(const reapNetIterator *iterator, reapNetResult *result)
{
    unsigned int local_addr, remote_addr, local_port, remote_port;
    unsigned long inode_long;
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
            EMIT_ERROR("Failed to read from /proc/net/%sp", iterator->tcp ? "tc" : "ud");
            return REAP_RET_OTHER;
        }
        else {
            return REAP_RET_DONE;
        }
    }

    if (sscanf(line, " %*u: %x:%x %x:%x %*s %*s %*s %*s %*u %*u %lu", &local_addr, &local_port, &remote_addr,
               &remote_port, &inode_long) != 5) {
#ifdef REAP_USE_ERROR_BUFFER
        unsigned int len;

        len = strnlen(line, sizeof(line));
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        EMIT_ERROR("Malformed line in /proc/net/%sp: %s", iterator->tcp ? "tc" : "ud", line);
#endif
        return REAP_RET_OTHER;
    }

    storeU32(local_addr, result->local.address);
    result->local.port = local_port;

    storeU32(remote_addr, result->remote.address);
    result->remote.port = remote_port;

    result->inode = inode_long;

    return REAP_RET_OK;
}

int
reapNet6IteratorInit(reapNet6Iterator *iterator, bool tcp)
{
    char buffer[20];

    if (!iterator) {
        EMIT_ERROR("The iterator cannot be NULL");
        return REAP_RET_BAD_USAGE;
    }

    iterator->tcp = tcp;
    snprintf(buffer, sizeof(buffer), "/proc/net/%sp6", tcp ? "tc" : "ud");
    return iteratorInit(&iterator->file, buffer);
}

void
reapNet6IteratorClose(reapNet6Iterator *iterator)
{
    if (iterator) {
        iteratorClose(&iterator->file);
    }
}

int
reapNet6IteratorNext(const reapNet6Iterator *iterator, reapNet6Result *result)
{
    unsigned int local_port, remote_port;
    unsigned long inode_long;
    char line[256], local_addr[33], remote_addr[33];

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
            EMIT_ERROR("Failed to read from /proc/net/%sp", iterator->tcp ? "tc" : "ud");
            return REAP_RET_OTHER;
        }
        else {
            return REAP_RET_DONE;
        }
    }

    if (sscanf(line, " %*u: %32s:%x %32s:%x %*s %*s %*s %*s %*u %*u %lu", local_addr, &local_port,
               remote_addr, &remote_port, &inode_long) != 5) {
#ifdef REAP_USE_ERROR_BUFFER
        unsigned int len;

        len = strnlen(line, sizeof(line));
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        EMIT_ERROR("Malformed line in /proc/net/%sp6: %s", iterator->tcp ? "tc" : "ud", line);
#endif
        return REAP_RET_OTHER;
    }

    populateAddr6(result->local.address, local_addr);
    result->local.port = local_port;

    populateAddr6(result->remote.address, remote_addr);
    result->remote.port = remote_port;

    result->inode = inode_long;

    return REAP_RET_OK;
}
