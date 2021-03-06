/**
 * @file iterate_map.h
 * @author Daniel Walker
 * @brief Exposes process memory map iteration.
 * @copyright Copyright (c) 2022
 *
 */

#ifndef REAP_ITERATE_MAP_H
#define REAP_ITERATE_MAP_H

#include <stdio.h>
#include <sys/mman.h>

#include "definitions.h"

/**
 * @brief Iterates over all line in a /proc/<PID>/maps file.
 *
 * @note User code should not access the iterator's fields.
 */
typedef struct reapMapIterator {
    FILE *file;
} reapMapIterator;

/**
 * @brief Result generated by a reapMapIterator.
 */
typedef struct reapMapResult {
    unsigned long start;             /**< The start of the memory section.*/
    unsigned long end;               /**< The end of the memory section.*/
    unsigned int offset;             /**< The offset of the memory section in the referent file.*/
    int permissions;                 /**< The permissions of the memory section.*/
    dev_t device;                    /**< The device number of the referent file.*/
    ino_t inode;                     /**< The inode of the referent file.*/
    char name[REAP_SHORT_PATH_SIZE]; /**< The name of the referent file.*/
} reapMapResult;

/**
 * @brief Initializes an iterator.
 *
 * @param pid       The PID.
 * @param iterator  A pointer to the iterator.
 *
 * @return          REAP_RET_Ok if successful and an error code otherwise.
 */
int
reapMapIteratorInit(pid_t pid, reapMapIterator *iterator);

/**
 * @brief Closes an iterator.
 *
 * @param iterator A pointer to the iterator.
 */
void
reapMapIteratorClose(reapMapIterator *iterator);

/**
 * @brief Gets the next result.
 *
 * @param iterator      A pointer to the iterator.
 * @param[out] result   A pointer to the result to be populated.
 *
 * @return              REAP_RET_OK if a result was found, REAP_RET_DONE if the iteration is finished, and an
 * error code otherwise.
 */
int
reapMapIteratorNext(const reapMapIterator *iterator, reapMapResult *result);

#endif  // REAP_ITERATE_MAP_H
