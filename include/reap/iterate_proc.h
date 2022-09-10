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

#include "definitions.h"

/**
 * @brief Iterates over the running processes.
 */
typedef struct reapProcIterator reapProcIterator;

/**
 * @brief Initializes an iterator.
 *
 * @param[out] iterator     A pointer to the pointer to be initialized.
 *
 * @return                  REAP_RET_OK if successful and an error code otherwise.
 */
int
reapProcIteratorCreate(reapProcIterator **iterator);

/**
 * @brief Destroys an iterator.
 *
 * @param iterator  A pointer to the iterator.
 */
void
reapProcIteratorDestroy(reapProcIterator *iterator);

/**
 * @brief Gets the next result.
 *
 * @param iterator          A pointer to the iterator.
 * @param[out] info         The result to be populated.
 * @param[out] exe_path     If not NULL, then will be populated by the path to the process' executable.
 * @param path_size         The size of exe_path.
 *
 * @return                  REAP_RET_OK if a result was found, REAP_RET_DONE if the iteration is finished,
 * and an error code otherwise.
 */
int
reapProcIteratorNext(const reapProcIterator *iterator, reapProcInfo *info, char *exe_path, size_t path_size);

#endif  // REAP_ITERATE_PROC_H
