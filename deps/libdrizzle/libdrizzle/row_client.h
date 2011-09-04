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
 * @brief Row Declarations for Clients
 */

#ifndef __DRIZZLE_ROW_CLIENT_H
#define __DRIZZLE_ROW_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup drizzle_row_client Row Declarations for Clients
 * @ingroup drizzle_client_interface
 *
 * These functions allow you to access rows in a result set. If the result is
 * unbuffered, you can read and buffer rows one at a time. If the rows are
 * buffered in the result, the drizzle_row_next() and related functions can be
 * used.
 * @{
 */

/**
 * Get next row number for unbuffered results. Use the drizzle_field* functions
 * to read individual fields after this function succeeds.
 */
DRIZZLE_API
uint64_t drizzle_row_read(drizzle_result_st *result, drizzle_return_t *ret_ptr);

/**
 * Read and buffer one row. The returned row must be freed by the caller with
 * drizzle_row_free().
 *
 * @param[in,out] result pointer to the result structure to read from.
 * @param[out] ret_pointer Standard drizzle return value.
 * @return the row that was read, or NULL if there are no more rows.
 */
DRIZZLE_API
drizzle_row_t drizzle_row_buffer(drizzle_result_st *result,
                                 drizzle_return_t *ret_ptr);

/**
 * Free a row that was buffered with drizzle_row_buffer().
 */
DRIZZLE_API
void drizzle_row_free(drizzle_result_st *result, drizzle_row_t row);

/**
 * Get an array of all field sizes for buffered rows.
 */
DRIZZLE_API
size_t *drizzle_row_field_sizes(drizzle_result_st *result);

/**
 * Get next buffered row from a fully buffered result.
 */
DRIZZLE_API
drizzle_row_t drizzle_row_next(drizzle_result_st *result);

/**
 * Get previous buffered row from a fully buffered result.
 */
DRIZZLE_API
drizzle_row_t drizzle_row_prev(drizzle_result_st *result);

/**
 * Seek to the given buffered row in a fully buffered result.
 */
DRIZZLE_API
void drizzle_row_seek(drizzle_result_st *result, uint64_t row);

/**
 * Get the given buffered row from a fully buffered result.
 */
DRIZZLE_API
drizzle_row_t drizzle_row_index(drizzle_result_st *result, uint64_t row);

/**
 * Get current row number.
 */
DRIZZLE_API
uint64_t drizzle_row_current(drizzle_result_st *result);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __DRIZZLE_ROW_CLIENT_H */
