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
 * @brief Column Declarations
 */

#ifndef __DRIZZLE_COLUMN_H
#define __DRIZZLE_COLUMN_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup drizzle_column Column Declarations
 * @ingroup drizzle_client_interface
 * @ingroup drizzle_server_interface
 *
 * These functions are used to get detailed column information. This information
 * is usually sent as the first part of a result set. There are multiple ways
 * for column information to be buffered depending on the functions being used.
 * @{
 */

/**
 * Initialize a column structure.
 */
DRIZZLE_API
drizzle_column_st *drizzle_column_create(drizzle_result_st *result,
                                         drizzle_column_st *column);

/**
 * Free a column structure.
 */
DRIZZLE_API
void drizzle_column_free(drizzle_column_st *column);

/**
 * Get the drizzle_result_st struct that the column belongs to.
 */
DRIZZLE_API
drizzle_result_st *drizzle_column_drizzle_result(drizzle_column_st *column);

/**
 * Get catalog name for a column.
 */
DRIZZLE_API
const char *drizzle_column_catalog(drizzle_column_st *column);

/**
 * Get database name for a column.
 */
DRIZZLE_API
const char *drizzle_column_db(drizzle_column_st *column);

/**
 * Get table name for a column.
 */
DRIZZLE_API
const char *drizzle_column_table(drizzle_column_st *column);

/**
 * Get original table name for a column.
 */
DRIZZLE_API
const char *drizzle_column_orig_table(drizzle_column_st *column);

/**
 * Get column name for a column.
 */
DRIZZLE_API
const char *drizzle_column_name(drizzle_column_st *column);

/**
 * Get original column name for a column.
 */
DRIZZLE_API
const char *drizzle_column_orig_name(drizzle_column_st *column);

/**
 * Get charset for a column.
 */
DRIZZLE_API
drizzle_charset_t drizzle_column_charset(drizzle_column_st *column);

/**
 * Get size of a column.
 */
DRIZZLE_API
uint32_t drizzle_column_size(drizzle_column_st *column);

/**
 * Get max size of a column.
 */
DRIZZLE_API
size_t drizzle_column_max_size(drizzle_column_st *column);

/**
 * Set max size of a column.
 */
DRIZZLE_API
void drizzle_column_set_max_size(drizzle_column_st *column, size_t size);

/**
 * Get the type of a column.
 */
DRIZZLE_API
drizzle_column_type_t drizzle_column_type(drizzle_column_st *column);

/**
 * Get the Drizzle type of a column.
 */
DRIZZLE_API
drizzle_column_type_drizzle_t
drizzle_column_type_drizzle(drizzle_column_st *column);

/**
 * Get flags for a column.
 */
DRIZZLE_API
drizzle_column_flags_t drizzle_column_flags(drizzle_column_st *column);

/**
 * Get the number of decimals for numeric columns.
 */
DRIZZLE_API
uint8_t drizzle_column_decimals(drizzle_column_st *column);

/**
 * Get default value for a column.
 */
DRIZZLE_API
const uint8_t *drizzle_column_default_value(drizzle_column_st *column,
                                            size_t *size);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __DRIZZLE_COLUMN_H */
