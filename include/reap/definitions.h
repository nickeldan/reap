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

/**
 * @brief REAP's version.
 */
#define REAP_VERSION "0.9.0"

/**
 * @brief Return values.
 */
enum reapRetValue {
    REAP_RET_OK,            /**< The function was successful.*/
    REAP_RET_DONE,          /**< The iteration is finished.*/
    REAP_RET_BAD_USAGE,     /**< A function was called with invalid arguments.*/
    REAP_RET_OUT_OF_MEMORY, /**< Memory failed to be allocated.*/
    REAP_RET_NOT_FOUND,     /**< A requested resource was not found.*/
    REAP_RET_FILE_READ,     /**< Failed to read from a file.*/
    REAP_RET_OTHER,         /**< Catch-all error.*/
};

/**
 * @brief Converts an error code to a string.
 *
 * @param value The error code.
 *
 * @return      A string describing the error.
 */
const char *
reapErrorString(int value)
#ifdef __GNUC__
    __attribute__((const))
#endif
    ;

#ifndef REAP_NO_ERROR_BUFFER
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
#endif

/**
 * @brief Information concerning a running process.
 */
typedef struct reapProcInfo {
    pid_t pid;  /**< Process ID.*/
    pid_t ppid; /**< Parent process ID.*/
    pid_t tgid; /**< Task group ID.*/
    uid_t uid;  /**< ID of the user running the process.*/
    uid_t euid; /**< Effective user ID running the process.*/
    gid_t gid;  /**< Group ID of the user running the process.*/
    gid_t egid; /**< Effective group ID running the process.*/
} reapProcInfo;

/**
 * @brief Gets information about a running process.
 *
 * @param pid               The PID of a running process.
 * @param[out] info         A pointer to the structure to be populated.
 * @param[out] exe_path     If not NULL, then will be populated by the path to the process' executable.
 * @param path_size         The size of exe_path.
 *
 * @return                  REAP_RET_OK if successful and an error code otherwise.
 */
int
reapGetProcInfo(pid_t pid, reapProcInfo *info, char *exe_path, size_t path_size);

#endif  // REAP_DEFINITIONS_H
