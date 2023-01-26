/**
 * @file proc.h
 * @author Daniel Walker
 * @brief Provides functionality for getting process info.
 *
 * @copyright Copyright (c) 2023
 */

#ifndef REAP_PROC_H
#define REAP_PROC_H

#include <sys/types.h>
#include <unistd.h>

#include "definitions.h"

/**
 * @brief   Acquires the thread-local buffer holding the last error message.
 *
 * @return  A pointer to the thread-local buffer.
 */
char *
reapGetError(void)
#ifdef __GNUC__
    __attribute__((pure))
#endif
    ;

/**
 * @brief Information concerning a running process/thread.
 */
typedef struct reapProcInfo {
    pid_t pid;  /**< Process ID.*/
    pid_t ppid; /**< Parent process ID.*/
    pid_t tid;  /**< Task ID.*/
    uid_t uid;  /**< ID of the user running the process.*/
    uid_t euid; /**< Effective user ID running the process.*/
    gid_t gid;  /**< Group ID of the user running the process.*/
    gid_t egid; /**< Effective group ID running the process.*/
} reapProcInfo;

/**
 * @brief Gets information about a running process/thread.
 *
 * @param pid               The PID of a running process (or TID for a thread).
 * @param[out] info         A pointer to the structure to be populated.
 * @param[out] exe_path     If not NULL, then will be populated by the path to the process' executable.
 * @param path_size         The size of exe_path.
 *
 * @return                  REAP_RET_OK if successful and an error code otherwise.
 */
int
reapGetProcInfo(pid_t pid, reapProcInfo *info, char *exe_path, size_t path_size);

#endif  // REAP_PROC_H
