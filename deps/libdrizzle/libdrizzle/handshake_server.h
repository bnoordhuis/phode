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
 * @brief Handshake Declarations for Servers
 */

#ifndef __DRIZZLE_HANDSHAKE_SERVER_H
#define __DRIZZLE_HANDSHAKE_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup drizzle_handshake_server Handshake Declarations for Servers
 * @ingroup drizzle_server_interface
 *
 * These functions are used to send and receive handshake packets in a server.
 * @{
 */

/**
 * Write server handshake packet to a client.
 *
 * @param[in] con Connection structure previously initialized with
 *  drizzle_con_create(), drizzle_con_clone(), or related functions.
 * @return Standard drizzle return value.
 */
DRIZZLE_API
drizzle_return_t drizzle_handshake_server_write(drizzle_con_st *con);

/**
 * Read handshake packet from the client in a server.
 *
 * @param[in] con Connection structure previously initialized with
 *  drizzle_con_create(), drizzle_con_clone(), or related functions.
 * @return Standard drizzle return value.
 */
DRIZZLE_API
drizzle_return_t drizzle_handshake_client_read(drizzle_con_st *con);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __DRIZZLE_HANDSHAKE_SERVER_H */
