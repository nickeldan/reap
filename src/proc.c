#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>

#include <reap/reap.h>

#include "internal.h"

int
reapGetProcInfo(pid_t pid, reapProcInfo *info, char *exe_path, size_t path_size)
{
    int ret = REAP_RET_OK;
    unsigned int num_found = 0;
    bool found_uid = false, found_gid = false, found_parent = false, found_tgid = false;
    char prefix[20], buffer[100], line[256];
    FILE *file;

    if (pid <= 0 || !info) {
        if (pid <= 0) {
            emitError("The PID must be positive");
        }
        else {
            emitError("The info cannot be NULL");
        }
        return REAP_RET_BAD_USAGE;
    }

    info->tid = pid;

    snprintf(prefix, sizeof(prefix), "/proc/%li", (long)pid);
    snprintf(buffer, sizeof(buffer), "%s/exe", prefix);

    if (exe_path && path_size > 0 && betterReadlink(buffer, exe_path, path_size) == -1) {
        int local_errno = errno;

        if (local_errno == ENOENT) {
            struct stat fs;

            if (lstat(buffer, &fs) == 0) {
                local_errno = EACCES;
            }
        }

        emitError("readlink failed on %s: %s", buffer, strerror(local_errno));
        return -1 * local_errno;
    }

    snprintf(buffer, sizeof(buffer), "%s/status", prefix);
    file = fopen(buffer, "r");
    if (!file) {
        emitError("%s not found", buffer);
        return REAP_RET_NOT_FOUND;
    }

    while (fgets(line, sizeof(line), file)) {
        unsigned long id, eid, ppid, tgid;

        if (!found_uid && sscanf(line, "Uid: %lu %lu %*u %*u\n", &id, &eid) == 2) {
            info->uid = id;
            info->euid = eid;
            found_uid = true;
        }
        else if (!found_gid && sscanf(line, "Gid: %lu %lu %*u %*u\n", &id, &eid) == 2) {
            info->gid = id;
            info->egid = eid;
            found_gid = true;
        }
        else if (!found_parent && sscanf(line, "PPid: %lu\n", &ppid) == 1) {
            info->ppid = ppid;
            found_parent = true;
        }
        else if (!found_tgid && sscanf(line, "Tgid: %lu\n", &tgid) == 1) {
            info->pid = tgid;
            found_tgid = true;
        }
        else {
            continue;
        }

        if (++num_found == 4) {
            goto done;
        }
    }

    emitError("No %s line found in %s", found_gid ? found_uid ? found_tgid ? "PPid" : "Tgid" : "Uid" : "Gid",
              buffer);
    ret = REAP_RET_OTHER;

done:
    fclose(file);
    return ret;
}
