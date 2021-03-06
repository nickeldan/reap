/**
 * @file iterate_proc.h
 * @author Daniel Walker
 * @brief Exposes process iteration.
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef REAP_ITERATE_PROC_H
#define REAP_ITERATE_PROC_H

#include <dirent.h>

#include "definitions.h"

/**
 * @brief Iterates over the running processes.
 *
 * @note User code should not access the iterator's fields.
 */
typedef struct reapProcIterator {
    DIR *dir;
} reapProcIterator;

/**
 * @brief Initializes an iterator.
 *
 * @param iterator  A pointer to the iterator.
 *
 * @return          REAP_RET_OK if successful and an error code otherwise.
 */
int
reapProcIteratorInit(reapProcIterator *iterator);

/**
 * @brief Closes an iterator.
 *
 * @param iterator  A pointer to the iterator.
 */
void
reapProcIteratorClose(reapProcIterator *iterator);

/**
 * @brief Gets the next result.
 *
 * @param iterator      A pointer to the iterator.
 * @param[out] info     The result to be populated.
 *
 * @return              REAP_RET_OK if a result was found, REAP_RET_DONE if the iteration is finished, and an
 * error code otherwise.
 */
int
reapProcIteratorNext(const reapProcIterator *iterator, reapProcInfo *info);

#endif  // REAP_ITERATE_PROC_H
