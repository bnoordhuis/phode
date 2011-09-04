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
 * @brief Result Declarations
 */

#ifndef __DRIZZLE_RESULT_H
#define __DRIZZLE_RESULT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup drizzle_result Result Declarations
 * @ingroup drizzle_client_interface
 * @ingroup drizzle_server_interface
 *
 * These are core result functions used by both clients and servers.
 * @{
 */

/**
 * Initialize a result structure.
 */
DRIZZLE_API
drizzle_result_st *drizzle_result_create(drizzle_con_st *con,
                                         drizzle_result_st *result);

/**
 * Clone a connection structure.
 */
DRIZZLE_API
drizzle_result_st *drizzle_result_clone(drizzle_con_st *con,
                                        drizzle_result_st *result,
                                        drizzle_result_st *from);

/**
 * Free a result structure.
 */
DRIZZLE_API
void drizzle_result_free(drizzle_result_st *result);

/**
 * Free all result structures.
 */
DRIZZLE_API
void drizzle_result_free_all(drizzle_con_st *con);

/**
 * Get the drizzle_con_st struct that the result belongs to.
 */
DRIZZLE_API
drizzle_con_st *drizzle_result_drizzle_con(drizzle_result_st *result);

/**
 * Get EOF flag for a result.
 */
DRIZZLE_API
bool drizzle_result_eof(drizzle_result_st *result);

/**
 * Get information string for a result.
 */
DRIZZLE_API
const char *drizzle_result_info(drizzle_result_st *result);

/**
 * Get error string for a result.
 */
DRIZZLE_API
const char *drizzle_result_error(drizzle_result_st *result);

/**
 * Get server defined error code for a result.
 */
DRIZZLE_API
uint16_t drizzle_result_error_code(drizzle_result_st *result);

/**
 * Get SQL state code for a result.
 */
DRIZZLE_API
const char *drizzle_result_sqlstate(drizzle_result_st *result);

/**
 * Get the number of warnings encounted during a command.
 */
DRIZZLE_API
uint16_t drizzle_result_warning_count(drizzle_result_st *result);

/**
 * Get inet ID of the last command, if any.
 */
DRIZZLE_API
uint64_t drizzle_result_insert_id(drizzle_result_st *result);

/**
 * Get the number of affected rows during the command.
 */
DRIZZLE_API
uint64_t drizzle_result_affected_rows(drizzle_result_st *result);

/**
 * Get the number of columns in a result set.
 */
DRIZZLE_API
uint16_t drizzle_result_column_count(drizzle_result_st *result);

/**
 * Get the number of rows returned for the command.
 */
DRIZZLE_API
uint64_t drizzle_result_row_count(drizzle_result_st *result);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __DRIZZLE_RESULT_H */
