/** * @file iterate_thread.h * @author Daniel Walker
 * @brief Exposes thread iteration.
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef REAP_ITERATE_THREAD_H
#define REAP_ITERATE_THREAD_H

#include <dirent.h>

#include "definitions.h"

/**
 * @brief Iterates over the threads of a process.
 *
 * @note User code should not access the iterator's fields.
 */
typedef struct reapThreadIterator {
    DIR *dir;
    pid_t pid;
} reapThreadIterator;

/**
 * @brief Initializes an iterator.
 *
 * @param pid       The PID whoses threads the iterator will return.
 * @param iterator  A pointer to the iterator.
 *
 * @return          REAP_RET_OK if successful and an error code otherwise.
 */
int
reapThreadIteratorInit(pid_t pid, reapThreadIterator *iterator);

/**
 * @brief Closes an iterator.
 *
 * @param iterator  A pointer to the iterator.
 */
void
reapThreadIteratorClose(reapThreadIterator *iterator);

/**
 * @brief Gets the next result.
 *
 * @param iterator      A pointer to the iterator.
 * @param[out] thread   The result to be populated with the thread's PID.
 *
 * @return              REAP_RET_OK if a result was found, REAP_RET_DONE if the iteration is finished, and an
 * error code otherwise.
 */
int
reapThreadIteratorNext(const reapThreadIterator *iterator, pid_t *thread);

#endif  // REAP_ITERATE_THREAD_H
