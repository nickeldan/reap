#include <stdbool.h>
#include <stdio.h>

#include <reap/reap.h>

#include "internal.h"

int
reapGetProcInfo(pid_t pid, reapProcInfo *info)
{
    int ret = REAP_RET_OK;
    bool found_uid = false, found_gid = false, found_parent = false;
    char prefix[20], buffer[100], line[256];
    FILE *file;

    if (pid <= 0 || !info) {
        return REAP_RET_BAD_USAGE;
    }

    info->pid = pid;

    snprintf(prefix, sizeof(prefix), "/proc/%li", (long)pid);
    snprintf(buffer, sizeof(buffer), "%s/exe", prefix);

    if (betterReadlink(buffer, info->exe, sizeof(info->exe)) == -1) {
        return translateErrno();
    }

    snprintf(buffer, sizeof(buffer), "%s/status", prefix);
    file = fopen(buffer, "r");
    if (!file) {
        return REAP_RET_NOT_FOUND;
    }

    while (fgets(line, sizeof(line), file)) {
        unsigned long id, eid, ppid;

        if (!found_uid && sscanf(line, "Uid: %lu %lu %*u %*u\n", &id, &eid) == 2) {
            info->uid = id;
            info->euid = eid;
            if (found_gid && found_parent) {
                goto done;
            }
            found_uid = true;
        }
        else if (!found_gid && sscanf(line, "Gid: %lu %lu %*u %*u\n", &id, &eid) == 2) {
            info->gid = id;
            info->egid = eid;
            if (found_uid && found_parent) {
                goto done;
            }
            found_gid = true;
        }
        else if (!found_parent && sscanf(line, "PPid: %lu\n", &ppid) == 1) {
            info->ppid = ppid;
            if (found_uid && found_gid) {
                goto done;
            }
            found_parent = true;
        }
    }

    ret = REAP_RET_OTHER;

done:
    fclose(file);
    return ret;
}
