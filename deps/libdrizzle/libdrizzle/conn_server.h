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
 * @brief Connection Declarations for Servers
 */

#ifndef __DRIZZLE_CON_SERVER_H
#define __DRIZZLE_CON_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup drizzle_con_server Connection Declarations for Servers
 * @ingroup drizzle_server_interface
 *
 * These functions extend the core connection functions with a set of functions
 * for server application use. These functions allow you to set raw handshake
 * information for use with the handshake write functions.
 * @{
 */

/**
 * Put a connection into listening mode.
 *
 * @param[in] con Connection structure previously initialized with
 *  drizzle_con_create(), drizzle_con_clone(), or related functions.
 * @return Standard drizzle return value.
 */
DRIZZLE_API
drizzle_return_t drizzle_con_listen(drizzle_con_st *con);

/**
 * Get connection backlog queue length.
 *
 * @param[in] con Connection structure previously initialized with
 *  drizzle_con_create(), drizzle_con_clone(), or related functions.
 * @return Backlog for connection
 */
DRIZZLE_API
int drizzle_con_backlog(const drizzle_con_st *con);

/**
 * Set connection backlog queue length.
 *
 * @param[in] con Connection structure previously initialized with
 *  drizzle_con_create(), drizzle_con_clone(), or related functions.
 * @param[in] backlog Backlog to use for connection
 */
DRIZZLE_API
void drizzle_con_set_backlog(drizzle_con_st *con, int backlog);

/**
 * Set protocol version for a connection.
 *
 * @param[in] con Connection structure previously initialized with
 *  drizzle_con_create(), drizzle_con_clone(), or related functions.
 * @param[in] protocol_version Protocol version to use for connection
 */
DRIZZLE_API
void drizzle_con_set_protocol_version(drizzle_con_st *con,
                                      uint8_t protocol_version);

/**
 * Set server version string for a connection.
 *
 * @param[in] con Connection structure previously initialized with
 *  drizzle_con_create(), drizzle_con_clone(), or related functions.
 * @param[in] server_version Server version to use for connection
 */
DRIZZLE_API
void drizzle_con_set_server_version(drizzle_con_st *con,
                                    const char *server_version);

/**
 * Set thread ID for a connection.
 *
 * @param[in] con Connection structure previously initialized with
 *  drizzle_con_create(), drizzle_con_clone(), or related functions.
 * @param[in] thread_id Thread ID to use for connection
 */
DRIZZLE_API
void drizzle_con_set_thread_id(drizzle_con_st *con, uint32_t thread_id);

/**
 * Set scramble buffer for a connection.
 *
 * @param[in] con Connection structure previously initialized with
 *  drizzle_con_create(), drizzle_con_clone(), or related functions.
 * @param[in] scramble Scramble to use for connection
 */
DRIZZLE_API
void drizzle_con_set_scramble(drizzle_con_st *con, const uint8_t *scramble);

/**
 * Set capabilities for a connection.
 *
 * @param[in] con Connection structure previously initialized with
 *  drizzle_con_create(), drizzle_con_clone(), or related functions.
 * @param[in] capabilities Capabilities to use for connection
 */
DRIZZLE_API
void drizzle_con_set_capabilities(drizzle_con_st *con,
                                  drizzle_capabilities_t capabilities);

/**
 * Set charset for a connection.
 *
 * @param[in] con Connection structure previously initialized with
 *  drizzle_con_create(), drizzle_con_clone(), or related functions.
 * @param[in] charset Character set to use for connection
 */
DRIZZLE_API
void drizzle_con_set_charset(drizzle_con_st *con, drizzle_charset_t charset);

/**
 * Set status for a connection.
 *
 * @param[in] con Connection structure previously initialized with
 *  drizzle_con_create(), drizzle_con_clone(), or related functions.
 * @param[in] status Status to use for connection
 */
DRIZZLE_API
void drizzle_con_set_status(drizzle_con_st *con, drizzle_con_status_t status);

/**
 * Set max packet size for a connection.
 *
 * @param[in] con Connection structure previously initialized with
 *  drizzle_con_create(), drizzle_con_clone(), or related functions.
 * @param[in] max_packet_size Max packet size to use for connection
 */
DRIZZLE_API
void drizzle_con_set_max_packet_size(drizzle_con_st *con,
                                     uint32_t max_packet_size);

/**
 * Copy all handshake information from one connection into another.
 *
 * @param[in] con Connection structure previously initialized with
 *  drizzle_con_create(), drizzle_con_clone(), or related functions.
 * @param[in] from Connection structure to copy from.
 */
DRIZZLE_API
void drizzle_con_copy_handshake(drizzle_con_st *con, drizzle_con_st *from);

/**
 * Read command without buffering.
 *
 * @param[in] con Connection structure previously initialized with
 *  drizzle_con_create(), drizzle_con_clone(), or related functions.
 * @param[out] command Command that was read.
 * @param[out] offset Where the data being returned begins in the command data.
 * @param[out] size The size of the data chunk being returned.
 * @param[out] total The total size of all command data being read.
 * @param[out] ret_ptr Standard drizzle return value.
 * @return On success, a pointer to an internal buffer with the command data.
 *  It will be *size bytes in length.
 */
DRIZZLE_API
void *drizzle_con_command_read(drizzle_con_st *con,
                               drizzle_command_t *command, size_t *offset,
                               size_t *size, size_t *total,
                               drizzle_return_t *ret_ptr);

/**
 * Read command and buffer it.
 *
 * @param[in] con Connection structure previously initialized with
 *  drizzle_con_create(), drizzle_con_clone(), or related functions.
 * @param[out] command Command that was read.
 * @param[out] total The total size of all command data being read.
 * @param[out] ret_ptr Standard drizzle return value.
 * @return On success, allocated buffer that holds the command data of length
 *  *total.
 */
DRIZZLE_API
void *drizzle_con_command_buffer(drizzle_con_st *con,
                                 drizzle_command_t *command, size_t *total,
                                 drizzle_return_t *ret_ptr);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __DRIZZLE_CON_SERVER_H */
