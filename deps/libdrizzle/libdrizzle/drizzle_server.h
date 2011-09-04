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
 * @brief Drizzle Declarations for Servers
 */

#ifndef __DRIZZLE_SERVER_H
#define __DRIZZLE_SERVER_H

#include <libdrizzle/drizzle.h>
#include <libdrizzle/conn_server.h>
#include <libdrizzle/handshake_server.h>
#include <libdrizzle/command_server.h>
#include <libdrizzle/result_server.h>
#include <libdrizzle/column_server.h>
#include <libdrizzle/row_server.h>
#include <libdrizzle/field_server.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup drizzle_server_interface Drizzle Server Interface
 */

/**
 * @addtogroup drizzle_server Drizzle Declarations for Servers
 * @ingroup drizzle_server_interface
 * @{
 */

/**
 * Add TCP (IPv4 or IPv6) connection for listening with common arguments.
 *
 * @param[in] drizzle Drizzle structure previously initialized with
 *  drizzle_create() or drizzle_clone().
 * @param[in] con Caller allocated structure, or NULL to allocate one.
 * @param[in] host Host to listen on. This may be a hostname to resolve, an
 *  IPv4 address, or an IPv6 address. This is passed directly to getaddrinfo().
 * @param[in] port Port to connect to.
 * @param[in] backlog Number of backlog connections passed to listen().
 * @param[in] options Drizzle connection options to add.
 * @return Same return as drizzle_con_create().
 */
DRIZZLE_API
drizzle_con_st *drizzle_con_add_tcp_listen(drizzle_st *drizzle,
                                           drizzle_con_st *con,
                                           const char *host, in_port_t port,
                                           int backlog,
                                           drizzle_con_options_t options);

/**
 * Add unix domain socket connection for listening with common arguments.
 *
 * @param[in] drizzle Drizzle structure previously initialized with
 *  drizzle_create() or drizzle_clone().
 * @param[in] con Caller allocated structure, or NULL to allocate one.
 * @param[in] uds Path to unix domain socket to use for listening.
 * @param[in] backlog Number of backlog connections passed to listen().
 * @param[in] options Drizzle connection options to add.
 * @return Same return as drizzle_con_create().
 */
DRIZZLE_API
drizzle_con_st *drizzle_con_add_uds_listen(drizzle_st *drizzle,
                                           drizzle_con_st *con,
                                           const char *uds, int backlog,
                                           drizzle_con_options_t options);

/**
 * Get next connection marked for listening that is ready for I/O.
 *
 * @param[in] drizzle Drizzle structure previously initialized with
 *  drizzle_create() or drizzle_clone().
 * @return Connection that is ready to accept, or NULL if there are none.
 */
DRIZZLE_API
drizzle_con_st *drizzle_con_ready_listen(drizzle_st *drizzle);

/**
 * Accept a new connection and initialize the connection structure for it.
 *
 * @param[in] drizzle Drizzle structure previously initialized with
 *  drizzle_create() or drizzle_clone().
 * @param[in] con Caller allocated structure, or NULL to allocate one.
 * @param[out] ret_ptr Standard drizzle return value.
 * @return Same return as drizzle_con_create().
 */
DRIZZLE_API
drizzle_con_st *drizzle_con_accept(drizzle_st *drizzle, drizzle_con_st *con,
                                   drizzle_return_t *ret_ptr);

/** @} */

#ifdef  __cplusplus
}
#endif

#endif /* __DRIZZLE_SERVER_H */
