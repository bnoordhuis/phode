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
 * @brief Column Declarations for Servers
 */

#ifndef __DRIZZLE_COLUMN_SERVER_H
#define __DRIZZLE_COLUMN_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup drizzle_column_server Column Declarations for Servers
 * @ingroup drizzle_server_interface
 *
 * These functions allow you to send column information over a connection.
 * @{
 */

/**
 * Write column information.
 */
DRIZZLE_API
drizzle_return_t drizzle_column_write(drizzle_result_st *result,
                                      drizzle_column_st *column);

/**
 * Set catalog name for a column.
 */
DRIZZLE_API
void drizzle_column_set_catalog(drizzle_column_st *column, const char *catalog);
 
/**
 * Set database name for a column.
 */
DRIZZLE_API
void drizzle_column_set_db(drizzle_column_st *column, const char *db);
 
/**
 * Set table name for a column.
 */
DRIZZLE_API
void drizzle_column_set_table(drizzle_column_st *column, const char *table);

/**
 * Set original table name for a column.
 */
DRIZZLE_API
void drizzle_column_set_orig_table(drizzle_column_st *column,
                                   const char *orig_table);

/**
 * Set column name for a column.
 */
DRIZZLE_API
void drizzle_column_set_name(drizzle_column_st *column, const char *name);

/**
 * Set original column name for a column.
 */
DRIZZLE_API
void drizzle_column_set_orig_name(drizzle_column_st *column,
                                  const char *orig_name);

/**
 * Set charset for a column.
 */
DRIZZLE_API
void drizzle_column_set_charset(drizzle_column_st *column,
                                drizzle_charset_t charset);

/**
 * Set size of a column.
 */
DRIZZLE_API
void drizzle_column_set_size(drizzle_column_st *column, uint32_t size);

/**
 * Set the type of a column.
 */
DRIZZLE_API
void drizzle_column_set_type(drizzle_column_st *column,
                             drizzle_column_type_t type);

/**
 * Set flags for a column.
 */
DRIZZLE_API
void drizzle_column_set_flags(drizzle_column_st *column,
                              drizzle_column_flags_t flags);

/**
 * Set the number of decimals for numeric columns.
 */
DRIZZLE_API
void drizzle_column_set_decimals(drizzle_column_st *column, uint8_t decimals);

/**
 * Set default value for a column.
 */
DRIZZLE_API
void drizzle_column_set_default_value(drizzle_column_st *column,
                                      const uint8_t *default_value,
                                      size_t size);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __DRIZZLE_COLUMN_SERVER_H */
