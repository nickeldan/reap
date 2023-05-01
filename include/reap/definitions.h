/**
 * @file definitions.h
 * @author Daniel Walker
 * @brief Provides library-wide definitions.
 *
 * @copyright Copyright (c) 2022
 */

#ifndef REAP_DEFINITIONS_H
#define REAP_DEFINITIONS_H

#include "config.h"

/**
 * @brief REAP's version.
 */
#define REAP_VERSION "1.0.1"

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

#endif  // REAP_DEFINITIONS_H
