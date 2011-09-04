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
 * @brief Connection Declarations for Clients
 */

#ifndef __DRIZZLE_CON_CLIENT_H
#define __DRIZZLE_CON_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup drizzle_con_client Connection Declarations for Clients
 * @ingroup drizzle_client_interface
 * @{
 */

/**
 * Connect to server.
 *
 * @param[in] con Connection structure previously initialized with
 *  drizzle_con_create(), drizzle_con_clone(), or related functions.
 * @return Standard drizzle return value.
 */
DRIZZLE_API
drizzle_return_t drizzle_con_connect(drizzle_con_st *con);

/**
 * Send quit command to server for a connection.
 *
 * @param[in] con Connection structure previously initialized with
 *  drizzle_con_create(), drizzle_con_clone(), or related functions.
 * @param[in] result Caller allocated structure, or NULL to allocate one.
 * @param[out] ret_ptr Standard drizzle return value.
 * @return On success, a pointer to the (possibly allocated) structure. On
 *  failure this will be NULL.
 */
DRIZZLE_API
drizzle_result_st *drizzle_con_quit(drizzle_con_st *con,
                                    drizzle_result_st *result,
                                    drizzle_return_t *ret_ptr);

/**
 * @todo Remove this with next major API change.
 */
DRIZZLE_API
drizzle_result_st *drizzle_quit(drizzle_con_st *con,
                                drizzle_result_st *result,
                                drizzle_return_t *ret_ptr);

/**
 * Select a new default database for a connection.
 *
 * @param[in] con Connection structure previously initialized with
 *  drizzle_con_create(), drizzle_con_clone(), or related functions.
 * @param[in] result Caller allocated structure, or NULL to allocate one.
 * @param[in] db Default database to select.
 * @param[out] ret_ptr Standard drizzle return value.
 * @return On success, a pointer to the (possibly allocated) structure. On
 *  failure this will be NULL.
 */
DRIZZLE_API
drizzle_result_st *drizzle_con_select_db(drizzle_con_st *con,
                                         drizzle_result_st *result,
                                         const char *db,
                                         drizzle_return_t *ret_ptr);

/**
 * @todo Remove this with next major API change.
 */
DRIZZLE_API
drizzle_result_st *drizzle_select_db(drizzle_con_st *con,
                                     drizzle_result_st *result,
                                     const char *db,
                                     drizzle_return_t *ret_ptr);

/**
 * Send a shutdown message to the server.
 *
 * @param[in] con Connection structure previously initialized with
 *  drizzle_con_create(), drizzle_con_clone(), or related functions.
 * @param[in] result Caller allocated structure, or NULL to allocate one.
 * @param[out] ret_ptr Standard drizzle return value.
 * @return On success, a pointer to the (possibly allocated) structure. On
 *  failure this will be NULL.
 */
DRIZZLE_API
drizzle_result_st *drizzle_con_shutdown(drizzle_con_st *con,
                                        drizzle_result_st *result,
                                        drizzle_return_t *ret_ptr);

/**
 * @todo Remove this with next major API change.
 */
#define DRIZZLE_SHUTDOWN_DEFAULT 0
DRIZZLE_API
drizzle_result_st *drizzle_shutdown(drizzle_con_st *con,
                                    drizzle_result_st *result, uint32_t level,
                                    drizzle_return_t *ret_ptr);

/**
 * Send a ping request to the server.
 *
 * @param[in] con Connection structure previously initialized with
 *  drizzle_con_create(), drizzle_con_clone(), or related functions.
 * @param[in] result Caller allocated structure, or NULL to allocate one.
 * @param[out] ret_ptr Standard drizzle return value.
 * @return On success, a pointer to the (possibly allocated) structure. On
 *  failure this will be NULL.
 */
DRIZZLE_API
drizzle_result_st *drizzle_con_ping(drizzle_con_st *con,
                                    drizzle_result_st *result,
                                    drizzle_return_t *ret_ptr);

/**
 * @todo Remove this with next major API change.
 */
DRIZZLE_API
drizzle_result_st *drizzle_ping(drizzle_con_st *con,
                                drizzle_result_st *result,
                                drizzle_return_t *ret_ptr);

/**
 * Send raw command to server, possibly in parts.
 *
 * @param[in] con Connection structure previously initialized with
 *  drizzle_con_create(), drizzle_con_clone(), or related functions.
 * @param[in] result Caller allocated structure, or NULL to allocate one.
 * @param[in] command Command to run on server.
 * @param[in] data Data to send along with the command.
 * @param[in] size Size of the current chunk of data being sent.
 * @param[in] total Total size of all data being sent for command.
 * @param[out] ret_ptr Standard drizzle return value.
 * @return On success, a pointer to the (possibly allocated) structure. On
 *  failure this will be NULL.
 */
DRIZZLE_API
drizzle_result_st *drizzle_con_command_write(drizzle_con_st *con,
                                             drizzle_result_st *result,
                                             drizzle_command_t command,
                                             const void *data, size_t size,
                                             size_t total,
                                             drizzle_return_t *ret_ptr);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __DRIZZLE_CON_CLIENT_H */
