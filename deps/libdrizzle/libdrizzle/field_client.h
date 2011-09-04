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
 * @brief Field Declarations for Clients
 */

#ifndef __DRIZZLE_FIELD_CLIENT_H
#define __DRIZZLE_FIELD_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup drizzle_field_client Field Declarations for Clients
 * @ingroup drizzle_client_interface
 *
 * These functions allow you to access fields in a result set if the result is
 * unbuffered. If the result is buffered, you can access the fields through the
 * row functions.
 * @{
 */

/**
 * Read field for unbuffered result, possibly in parts. This is especially
 * useful for blob streaming, since the client does not need to buffer the
 * entire blob.
 */
DRIZZLE_API
drizzle_field_t drizzle_field_read(drizzle_result_st *result, size_t *offset,
                                   size_t *size, size_t *total,
                                   drizzle_return_t *ret_ptr);

/**
 * Buffer one field.
 */
DRIZZLE_API
drizzle_field_t drizzle_field_buffer(drizzle_result_st *result, size_t *total,
                                     drizzle_return_t *ret_ptr);

/**
 * Free a buffered field.
 */
DRIZZLE_API
void drizzle_field_free(drizzle_field_t field);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __DRIZZLE_FIELD_CLIENT_H */
