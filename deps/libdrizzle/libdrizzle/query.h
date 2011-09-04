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
 * @brief Query Declarations
 */

#ifndef __DRIZZLE_QUERY_H
#define __DRIZZLE_QUERY_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup drizzle_query Query Declarations
 *
 * @ingroup drizzle_client_interface
 * These functions are used to issue queries on a connection. Single queries are
 * made using the drizzle_query function, or you can queue multiple queries and
 * run them concurrently using the other query functions.
 * @{
 */

/**
 * Send query to server. A \ref drizzle_result_st will be created for the
 * results.
 *
 * @param[in] con connection to use to send the query.
 * @param[in,out] result pointer to an unused structure that will be used for
 *                       the results, or NULL to allocate a new structure.
 * @param[in] query query string to send.
 * @param[in] size length of the query string in bytes.
 * @param[out] ret_ptr pointer to the result code.
 * @return result, a pointer to the newly allocated result structure, or NULL
 *         if the allocation failed.
 */
DRIZZLE_API
drizzle_result_st *drizzle_query(drizzle_con_st *con, drizzle_result_st *result,
                                 const char *query, size_t size,
                                 drizzle_return_t *ret_ptr);

/**
 * Send query to server, using strlen to get the size of query buffer..
 */
DRIZZLE_API
drizzle_result_st *drizzle_query_str(drizzle_con_st *con,
                                     drizzle_result_st *result,
                                     const char *query,
                                     drizzle_return_t *ret_ptr);

/**
 * Send query incrementally.
 */
DRIZZLE_API
drizzle_result_st *drizzle_query_inc(drizzle_con_st *con,
                                     drizzle_result_st *result,
                                     const char *query, size_t size,
                                     size_t total, drizzle_return_t *ret_ptr);

/**
 * Add a query to be run concurrently.
 */
DRIZZLE_API
drizzle_query_st *drizzle_query_add(drizzle_st *drizzle,
                                    drizzle_query_st *query,
                                    drizzle_con_st *con,
                                    drizzle_result_st *result,
                                    const char *query_string, size_t size,
                                    drizzle_query_options_t options,
                                    void *context);

/**
 * Initialize a query structure.
 */
DRIZZLE_API
drizzle_query_st *drizzle_query_create(drizzle_st *drizzle,
                                       drizzle_query_st *query);

/**
 * Free a query structure.
 */
DRIZZLE_API
void drizzle_query_free(drizzle_query_st *query);

/**
 * Free a query structure.
 */
DRIZZLE_API
void drizzle_query_free_all(drizzle_st *drizzle);

/**
 * Get connection struct for a query.
 */
DRIZZLE_API
drizzle_con_st *drizzle_query_con(drizzle_query_st *query);

/**
 * Set connection struct for a query.
 */
DRIZZLE_API
void drizzle_query_set_con(drizzle_query_st *query, drizzle_con_st *con);

/**
 * Get result struct for a query.
 */
DRIZZLE_API
drizzle_result_st *drizzle_query_result(drizzle_query_st *query);

/**
 * Set result struct for a query.
 */
DRIZZLE_API
void drizzle_query_set_result(drizzle_query_st *query,
                              drizzle_result_st *result);

/**
 * Get query string for a query.
 */
DRIZZLE_API
char *drizzle_query_string(drizzle_query_st *query, size_t *size);

/**
 * Set query string for a query.
 */
DRIZZLE_API
void drizzle_query_set_string(drizzle_query_st *query, const char *string,
                              size_t size);

/**
 * Get options for a query. 
 */
DRIZZLE_API
drizzle_query_options_t drizzle_query_options(drizzle_query_st *query);

/**
 * Set options for a query.
 */
DRIZZLE_API
void drizzle_query_set_options(drizzle_query_st *query,
                               drizzle_query_options_t options);

/**
 * Add options for a query.
 */
DRIZZLE_API
void drizzle_query_add_options(drizzle_query_st *query,
                               drizzle_query_options_t options);

/**
 * Remove options for a query.
 */
DRIZZLE_API
void drizzle_query_remove_options(drizzle_query_st *query,
                                  drizzle_query_options_t options);

/**
 * Get application context for a query.
 */
DRIZZLE_API
void *drizzle_query_context(drizzle_query_st *query);

/**
 * Set application context for a query.
 */
DRIZZLE_API
void drizzle_query_set_context(drizzle_query_st *query, void *context);

/**
 * Set callback function when the context pointer should be freed.
 */
DRIZZLE_API
void drizzle_query_set_context_free_fn(drizzle_query_st *query,
                                       drizzle_query_context_free_fn *function);

/**
 * Run queries concurrently, returning when one is complete.
 */
DRIZZLE_API
drizzle_query_st *drizzle_query_run(drizzle_st *drizzle,
                                    drizzle_return_t *ret_ptr);

/**
 * Run queries until they are all complete. Returns \ref DRIZZLE_RETURN_OK if
 * all queries complete, even if some return errors. This returns immediately
 * if some other error occurs, leaving some queries unprocessed. You must call
 * drizzle_result_error_code() to check if each query succeeded.
 */
DRIZZLE_API
drizzle_return_t drizzle_query_run_all(drizzle_st *drizzle);

/*
 * Escape a string or encode a string in hexadecimal. The return value is the
 * size of the output string in to.
 */
DRIZZLE_API
size_t drizzle_escape_string(char *to, const char *from, size_t from_size);
DRIZZLE_API
size_t drizzle_hex_string(char *to, const char *from, size_t from_size);
DRIZZLE_API
void drizzle_mysql_password_hash(char *to, const char *from, size_t from_size);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __DRIZZLE_QUERY_H */
