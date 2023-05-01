/**
 * @file config.h
 * @author Daniel Walker
 * brief Contains configuration parameter(s).
 *
 * @copyright Copyright (c) 2023
 */

#ifndef REAP_CONFIG_H
#define REAP_CONFIG_H

#ifndef REAP_ERROR_BUFFER_SIZE
/**
 * @brief Size of the buffer returned by reapGetError.
 */
#define REAP_ERROR_BUFFER_SIZE 256
#endif

#ifdef REAP_NO_PROC
#ifndef REAP_NO_ITERATE_PROC
#define REAP_NO_ITERATE_PROC
#endif
#endif

#endif  // REAP_CONFIG_H
