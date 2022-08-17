/**
 * @file definitions.h
 * @author Daniel Walker
 * @brief Provides library-wide definitions.
 *
 * @copyright Copyright (c) 2022
 */

#ifndef REAP_DEFINITIONS_H
#define REAP_DEFINITIONS_H

#include <sys/types.h>
#include <unistd.h>

#include "config.h"

/**
 * @brief REAP's version.
 */
#define REAP_VERSION "0.8.4"

/**
 * @brief Return values.
 */
enum reapRetValue {
    REAP_RET_OK,             /**< The function was successful.*/
    REAP_RET_DONE,           /**< The iteration is finished.*/
    REAP_RET_BAD_USAGE,      /**< A funciton was called with invalid arguments.*/
    REAP_RET_OUT_OF_MEMORY,  /**< A memory allocation failed.*/
    REAP_RET_NO_PERMISSION,  /**< No permission to access a requested resource.*/
    REAP_RET_NOT_FOUND,      /**< A requested resource was not found.*/
    REAP_RET_TOO_MANY_LINKS, /**< Too many symbolic links encountered.*/
    REAP_RET_FILE_READ,      /**< Failed to read from a file.*/
    REAP_RET_OTHER,          /**< Catch-all error.*/
};

/**
 * @brief Converts an error code to a string.
 *
 * @param value The error code.
 *
 * @return      A string describing the error.
 */
const char *
reapErrorString(int value) __attribute__((pure));

#ifdef REAP_USE_ERROR_BUFFER
/**
 * @brief   Acquires the thread-local buffer holding the last error message.
 *
 * @return  A pointer to the thread-local buffer.
 */
char *
reapGetError(void);
#endif

/**
 * @brief Information concerning a running process.
 */
typedef struct reapProcInfo {
    pid_t pid;                      /**< Process ID.*/
    pid_t ppid;                     /**< Parent process ID.*/
    pid_t tgid;                     /**< Task group ID.*/
    uid_t uid;                      /**< ID of the user running the process.*/
    uid_t euid;                     /**< Effective user ID running the process.*/
    gid_t gid;                      /**< Group ID of the user running the process.*/
    gid_t egid;                     /**< Effective group ID running the process.*/
    char exe[REAP_SHORT_PATH_SIZE]; /**< Path of the file being run.*/
} reapProcInfo;

/**
 * @brief Gets information about a running process.
 *
 * @param pid       The PID of a running process.
 * @param[out] info A pointer to the structure to be populated.
 *
 * @return          REAP_RET_OK if successful and an error code otherwise.
 */
int
reapGetProcInfo(pid_t pid, reapProcInfo *info);

#endif  // REAP_DEFINITIONS_H
