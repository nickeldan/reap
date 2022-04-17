#ifndef REAP_DEFINITIONS_H
#define REAP_DEFINITIONS_H

#include <sys/types.h>
#include <unistd.h>

#include "config.h"

enum reapRetValue {
    REAP_RET_OK,
    REAP_RET_DONE,
    REAP_RET_BAD_USAGE,
    REAP_RET_OUT_OF_MEMORY,
    REAP_RET_NO_PERMISSION,
    REAP_RET_NOT_FOUND,
    REAP_RET_TOO_MANY_LINKS,
    REAP_RET_OTHER,
};

const char *
reapErrorString(int value) __attribute__((pure));

#ifdef REAP_USE_ERROR_BUFFER
char *
reapGetError(void);
#endif

typedef struct reapProcInfo {
    pid_t pid;
    pid_t ppid;
    uid_t uid;
    uid_t euid;
    gid_t gid;
    gid_t egid;
    char exe[REAP_SHORT_PATH_SIZE];
} reapProcInfo;

int
reapGetProcInfo(pid_t pid, reapProcInfo *info);

#endif  // REAP_DEFINITIONS_H
