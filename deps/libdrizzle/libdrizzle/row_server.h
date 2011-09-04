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
 * @brief Row Declarations for Servers
 */

#ifndef __DRIZZLE_ROW_SERVER_H
#define __DRIZZLE_ROW_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup drizzle_row_server Row Declarations for Servers
 * @ingroup drizzle_server_interface
 *
 * These functions allow you to send row information over a connection.
 * @{
 */

/**
 * Write next row.
 */
DRIZZLE_API
drizzle_return_t drizzle_row_write(drizzle_result_st *result);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __DRIZZLE_ROW_SERVER_H */
