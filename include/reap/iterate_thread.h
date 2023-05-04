/**
 * @file iterate_thread.h * @author Daniel Walker
 * @brief Exposes thread iteration.
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef REAP_ITERATE_THREAD_H
#define REAP_ITERATE_THREAD_H

#include "definitions.h"

#ifndef REAP_NO_ITERATE_THREAD

/**
 * @brief Iterates over the threads of a process.
 */
typedef struct reapThreadIterator reapThreadIterator;

/**
 * @brief Creates a thread iterator.
 *
 * @param pid               The PID whoses threads the iterator will return.
 * @param iterator[out]     A pointer to the pointer to be initialized.
 *
 * @return                  REAP_RET_OK if successful and an error code otherwise.
 */
int
reapThreadIteratorCreate(pid_t pid, reapThreadIterator **iterator) REAP_EXPORT;

/**
 * @brief Destroys an iterator.
 *
 * @param iterator  A pointer to the iterator.
 */
void
reapThreadIteratorDestroy(reapThreadIterator *iterator) REAP_EXPORT;

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
reapThreadIteratorNext(const reapThreadIterator *iterator, pid_t *thread) REAP_EXPORT;

#endif  // REAP_NO_ITERATE_THREAD

#endif  // REAP_ITERATE_THREAD_H
