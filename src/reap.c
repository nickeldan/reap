#include <errno.h>
#include <linux/limits.h>
#include <stdio.h>
#include <sys/stat.h>

#include "reap.h"

static int
translateErrno(void)
{
    switch (errno) {
    case EACCES: return REAP_RET_NO_PERMISSION;
    case ELOOP: return REAP_RET_TOO_MANY_LINKS;
    case ENOENT:
    case ENOTDIR:
    case EIO: return REAP_RET_NOT_FOUND;
    case ENOMEM: return REAP_RET_OUT_OF_MEMORY;
    default: return REAP_RET_OTHER;
    }
}

bool
reapCheck(void)
{
    struct stat fs;

    return (stat("/proc/self", &fs) == 0 && S_ISDIR(fs.st_mode));
}

const char *
reapErrorString(int value)
{
    switch (value) {
    case REAP_RET_OK: return "No error";
    case REAP_RET_BAD_USAGE: return "Invalid API usage";
    case REAP_RET_OUT_OF_MEMORY: return "Failed to allocate memory";
    case REAP_RET_NO_PERMISSION: return "No permission";
    case REAP_RET_NOT_FOUND: return "Could not find a resource";
    case REAP_RET_TOO_MANY_LINKS: return "Too many symbolic links to resolve";
    case REAP_RET_OTHER: return "An unanticpated error occurred";
    default: return "Invalid return value";
    }
}

int
reapGetInfo(pid_t pid, reapInfo *info)
{
    int ret = REAP_RET_OK;
    bool found_uid = false, found_gid = false, found_parent = false;
    char prefix[20], buffer[PATH_MAX];
    FILE *file;

    if (pid <= 0 || !info) {
        return REAP_RET_BAD_USAGE;
    }

    snprintf(prefix, sizeof(prefix), "/proc/%li", (long)pid);
    snprintf(buffer, sizeof(buffer), "%s/exe", prefix);

    if (readlink(buffer, info->exe, sizeof(info->exe)) == -1) {
        return translateErrno();
    }

    snprintf(buffer, sizeof(buffer), "%s/status", prefix);
    file = fopen(buffer, "r");
    if (!file) {
        return REAP_RET_NOT_FOUND;
    }

    while (fgets(line, sizeof(line), file)) {
        unsigned long id, eid, ppid;

        if (!found_uuid && sscanf(line, "Uid: %lu %lu %*lu %*lu\n", &id, &eid) == 2) {
            info->uid = id;
            info->euid = eid;
            if (found_gid && found_parent) {
                goto done;
            }
            found_uid = true;
        }
        else if (!found_gid && sscanf(line, "Gid: %lu %lu %*lu %*lu\n", &id, &eid) == 2) {
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
