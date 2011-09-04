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
 * @brief Drizzle Declarations for Clients
 */

#ifndef __DRIZZLE_CLIENT_H
#define __DRIZZLE_CLIENT_H

#include <libdrizzle/drizzle.h>
#include <libdrizzle/conn_client.h>
#include <libdrizzle/handshake_client.h>
#include <libdrizzle/command_client.h>
#include <libdrizzle/query.h>
#include <libdrizzle/result_client.h>
#include <libdrizzle/column_client.h>
#include <libdrizzle/row_client.h>
#include <libdrizzle/field_client.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup drizzle_client_interface Drizzle Client Interface
 */

/**
 * @addtogroup drizzle_client Drizzle Declarations for Clients
 * @ingroup drizzle_client_interface
 * @{
 */

/**
 * Add TCP (IPv4 or IPv6) connection with common arguments.
 *
 * @param[in] drizzle Drizzle structure previously initialized with
 *  drizzle_create() or drizzle_clone().
 * @param[in] con Caller allocated structure, or NULL to allocate one.
 * @param[in] host Host to connect to. This may be a hostname to resolve, an
 *  IPv4 address, or an IPv6 address. This is passed directly to getaddrinfo().
 * @param[in] port Remote port to connect to.
 * @param[in] user User to use while establishing the connection.
 * @param[in] password Password to use while establishing the connection.
 * @param[in] db Initial database to connect to.
 * @param[in] options Drizzle connection options to add.
 * @return Same return as drizzle_con_create().
 */
DRIZZLE_API
drizzle_con_st *drizzle_con_add_tcp(drizzle_st *drizzle, drizzle_con_st *con,
                                    const char *host, in_port_t port,
                                    const char *user, const char *password,
                                    const char *db,
                                    drizzle_con_options_t options);

/**
 * Add unix domain socket connection with common arguments.
 *
 * @param[in] drizzle Drizzle structure previously initialized with
 *  drizzle_create() or drizzle_clone().
 * @param[in] con Caller allocated structure, or NULL to allocate one.
 * @param[in] uds Path to unix domain socket to use for connection.
 * @param[in] user User to use while establishing the connection.
 * @param[in] password Password to use while establishing the connection.
 * @param[in] db Initial database to connect to.
 * @param[in] options Drizzle connection options to add.
 * @return Same return as drizzle_con_create().
 */
DRIZZLE_API
drizzle_con_st *drizzle_con_add_uds(drizzle_st *drizzle, drizzle_con_st *con,
                                    const char *uds, const char *user,
                                    const char *password, const char *db,
                                    drizzle_con_options_t options);

/** @} */

#ifdef  __cplusplus
}
#endif

#endif /* __DRIZZLE_CLIENT_H */
