/**
 * @file iterate_fd.h
 * @author Daniel Walker
 * @brief Exposes file descriptor iteration.
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef REAP_ITERATE_FD_H
#define REAP_ITERATE_FD_H

#include <dirent.h>

#include "definitions.h"

/**
 * @brief Iterates over all open file descriptors of a process.
 *
 * @note User code should not access the iterator's fields.
 */
typedef struct reapFdIterator {
    DIR *dir;
    pid_t pid;
} reapFdIterator;

/**
 * @brief Result generated by a reapFdIterator.
 */
typedef struct reapFdResult {
    int fd;                          /**< The file descriptor.*/
    dev_t device;                    /**< The device number of the referent file.*/
    ino_t inode;                     /**< The inode of the referent file.*/
    mode_t mode;                     /**< The type and mode of the referent file.*/
    char file[REAP_SHORT_PATH_SIZE]; /**< The name of the referent file.*/
} reapFdResult;

/**
 * @brief Initializes an iterator.
 *
 * @param pid       The PID of the process whose descriptors the iterator will return.
 * @param iterator  A pointer to the iterator.
 *
 * @return          REAP_RET_OK if successful and an error code otherwise.
 */
int
reapFdIteratorInit(pid_t pid, reapFdIterator *iterator);

/**
 * @brief Closes an iterator.
 *
 * @param iterator  A pointer to the iterator.
 */
void
reapFdIteratorClose(reapFdIterator *iterator);

/**
 * @brief Gets the next result.
 *
 * @param iterator      A pointer to the iterator.
 * @param[out] result   The result to be populated.
 *
 * @return              REAP_RET_OK if successful and an error code otherwise.
 */
int
reapFdIteratorNext(const reapFdIterator *iterator, reapFdResult *result);

/**
 * @brief Gets the name of a file associated with a file descriptor.
 *
 * @param pid       The PID of the process.
 * @param fd        The file descriptor.
 * @param[out] dest A buffer where the file name will be placed.
 * @param size      The size of the buffer.
 *
 * @return          REAP_RET_OK if a result was found, REAP_RET_DONE if the iteration is finished, and an
 * error code otherwise.
 */
int
reapReadFd(pid_t pid, int fd, char *dest, size_t size);

#endif  // REAP_ITERATE_FD_H
