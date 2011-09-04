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
 * @brief Handshake Declarations for Clients
 */

#ifndef __DRIZZLE_HANDSHAKE_CLIENT_H
#define __DRIZZLE_HANDSHAKE_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup drizzle_handshake_client Handshake Declarations for Clients
 * @ingroup drizzle_client_interface
 *
 * These functions are used to send and receive handshake packets for a client.
 * These are only used by low-level clients when the DRIZZLE_CON_RAW_PACKET
 * option is set, so most applications will never need to use these.
 * @{
 */

/**
 * Read handshake packet from the server in a client.
 *
 * @param[in] con Connection structure previously initialized with
 *  drizzle_con_create(), drizzle_con_clone(), or related functions.
 * @return Standard drizzle return value.
 */
DRIZZLE_API
drizzle_return_t drizzle_handshake_server_read(drizzle_con_st *con);

/**
 * Write client handshake packet to a server.
 *
 * @param[in] con Connection structure previously initialized with
 *  drizzle_con_create(), drizzle_con_clone(), or related functions.
 * @return Standard drizzle return value.
 */
DRIZZLE_API
drizzle_return_t drizzle_handshake_client_write(drizzle_con_st *con);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __DRIZZLE_HANDSHAKE_CLIENT_H */
