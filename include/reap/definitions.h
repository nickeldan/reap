#ifndef REAP_DEFINITIONS_H
#define REAP_DEFINITIONS_H

#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>

#define SHORT_PATH_SIZE 256

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

typedef struct reapInfo {
    pid_t ppid;
    uid_t uid;
    uid_t euid;
    git_t gid;
    gid_t egid;
    char exe[SHORT_PATH_SIZE];
} reapInfo;

bool
reapCheck(void);

int
reapGetInfo(pid_t pid, reapInfo *info);

#endif  // REAP_DEFINITIONS_H
