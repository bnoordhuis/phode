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
 * @brief Result Declarations for Clients
 */

#ifndef __DRIZZLE_RESULT_CLIENT_H
#define __DRIZZLE_RESULT_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup drizzle_result_client Result Declarations for Clients
 * @ingroup drizzle_client_interface
 *
 * These functions read or buffer the result for a client command.
 * @{
 */

/**
 * Read result packet.
 */
DRIZZLE_API
drizzle_result_st *drizzle_result_read(drizzle_con_st *con,
                                       drizzle_result_st *result,
                                       drizzle_return_t *ret_ptr);

/**
 * Buffer all data for a result.
 */
DRIZZLE_API
drizzle_return_t drizzle_result_buffer(drizzle_result_st *result);

/**
 * Get result row packet size.
 */
DRIZZLE_API
size_t drizzle_result_row_size(drizzle_result_st *result);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __DRIZZLE_RESULT_CLIENT_H */
