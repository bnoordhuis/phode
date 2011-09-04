/*
 * Drizzle Client & Protocol Library
 *
 * Copyright (C) 2008 Eric Day (eday@oddments.org)
 * All rights reserved.
 *
 * Use and distribution licensed under the BSD license.  See
 * the COPYING file in this directory for full text.
 */

/**
 * @file
 * @brief Column Declarations for Clients
 */

#ifndef __DRIZZLE_COLUMN_CLIENT_H
#define __DRIZZLE_COLUMN_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup drizzle_column_client Column Declarations for Clients
 * @ingroup drizzle_client_interface
 *
 * These functions are used to get detailed column information. This information
 * is usually sent as the first part of a result set. There are both buffered
 * and unbuffered functions provided.
 * @{
 */

/**
 * Skip all columns in result.
 */
DRIZZLE_API
drizzle_return_t drizzle_column_skip(drizzle_result_st *result);

/**
 * Read column information.
 */
DRIZZLE_API
drizzle_column_st *drizzle_column_read(drizzle_result_st *result,
                                       drizzle_column_st *column,
                                       drizzle_return_t *ret_ptr);

/**
 * Buffer all columns in result structure.
 */
DRIZZLE_API
drizzle_return_t drizzle_column_buffer(drizzle_result_st *result);

/**
 * Get next buffered column from a result structure.
 */
DRIZZLE_API
drizzle_column_st *drizzle_column_next(drizzle_result_st *result);

/**
 * Get previous buffered column from a result structure.
 */
DRIZZLE_API
drizzle_column_st *drizzle_column_prev(drizzle_result_st *result);

/**
 * Seek to the given buffered column in a result structure.
 */
DRIZZLE_API
void drizzle_column_seek(drizzle_result_st *result, uint16_t column);

/**
 * Get the given buffered column from a result structure.
 */
DRIZZLE_API
drizzle_column_st *drizzle_column_index(drizzle_result_st *result,
                                        uint16_t column);

/**
 * Get current column number in a buffered or unbuffered result.
 */
DRIZZLE_API
uint16_t drizzle_column_current(drizzle_result_st *result);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __DRIZZLE_COLUMN_CLIENT_H */
