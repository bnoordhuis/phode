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
 * @brief Field Declarations for Servers
 */

#ifndef __DRIZZLE_FIELD_SERVER_H
#define __DRIZZLE_FIELD_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup drizzle_field_server Field Declarations for Servers
 * @ingroup drizzle_server_interface
 *
 * These functions allow you to send a field over a connection.
 * @{
 */

DRIZZLE_API
drizzle_return_t drizzle_field_write(drizzle_result_st *result,
                                     const drizzle_field_t field, size_t size,
                                     size_t total);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __DRIZZLE_FIELD_SERVER_H */
