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
 * @brief Packing Declarations
 */

#ifndef __DRIZZLE_PACK_H
#define __DRIZZLE_PACK_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup drizzle_pack Packing Declarations
 *
 * These functions are used internally to pack various parts of the protocol.
 * Not all functions are defined in pack.c, they are in the most appropriate
 * source file (for example, handshake.c for drizzle_pack_client_handshake).
 * @{
 */

/**
 * Pack length-encoded number.
 */
DRIZZLE_API
uint8_t *drizzle_pack_length(uint64_t number, uint8_t *ptr);

/**
 * Unpack length-encoded number.
 */
DRIZZLE_API
uint64_t drizzle_unpack_length(drizzle_con_st *con, drizzle_return_t *ret_ptr);

/**
 * Pack length-encoded string.
 */
DRIZZLE_API
uint8_t *drizzle_pack_string(char *string, uint8_t *ptr);

/**
 * Unpack length-encoded string.
 */
DRIZZLE_API
drizzle_return_t drizzle_unpack_string(drizzle_con_st *con, char *buffer,
                                       uint64_t max_size);

/**
 * Pack user, scramble, and db.
 */
DRIZZLE_API
uint8_t *drizzle_pack_auth(drizzle_con_st *con, uint8_t *ptr,
                           drizzle_return_t *ret_ptr);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __DRIZZLE_PACK_H */
