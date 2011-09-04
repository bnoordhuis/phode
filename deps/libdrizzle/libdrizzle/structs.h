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
 * @brief Struct Definitions
 */

#ifndef __DRIZZLE_STRUCTS_H
#define __DRIZZLE_STRUCTS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @ingroup drizzle
 */
struct drizzle_st
{
  uint16_t error_code;
  drizzle_options_t options;
  drizzle_verbose_t verbose;
  uint32_t con_count;
  uint32_t pfds_size;
  uint32_t query_count;
  uint32_t query_new;
  uint32_t query_running;
  int last_errno;
  int timeout;
  drizzle_con_st *con_list;
  void *context;
  drizzle_context_free_fn *context_free_fn;
  drizzle_event_watch_fn *event_watch_fn;
  void *event_watch_context;
  drizzle_log_fn *log_fn;
  void *log_context;
  struct pollfd *pfds;
  drizzle_query_st *query_list;
  char sqlstate[DRIZZLE_MAX_SQLSTATE_SIZE + 1];
  char last_error[DRIZZLE_MAX_ERROR_SIZE];
};

/**
 * @ingroup drizzle_con
 */
struct drizzle_con_tcp_st
{
  in_port_t port;
  struct addrinfo *addrinfo;
  char *host;
  char host_buffer[NI_MAXHOST];
};

/**
 * @ingroup drizzle_con
 */
struct drizzle_con_uds_st
{
  struct addrinfo addrinfo;
  struct sockaddr_un sockaddr;
};

/**
 * @ingroup drizzle_con
 */
struct drizzle_con_st
{
  uint8_t packet_number;
  uint8_t protocol_version;
  uint8_t state_current;
  short events;
  short revents;
  drizzle_capabilities_t capabilities;
  drizzle_charset_t charset;
  drizzle_command_t command;
  drizzle_con_options_t options;
  drizzle_con_socket_t socket_type;
  drizzle_con_status_t status;
  uint32_t max_packet_size;
  uint32_t result_count;
  uint32_t thread_id;
  int backlog;
  int fd;
  size_t buffer_size;
  size_t command_offset;
  size_t command_size;
  size_t command_total;
  size_t packet_size;
  struct addrinfo *addrinfo_next;
  uint8_t *buffer_ptr;
  uint8_t *command_buffer;
  uint8_t *command_data;
  void *context;
  drizzle_con_context_free_fn *context_free_fn;
  drizzle_st *drizzle;
  drizzle_con_st *next;
  drizzle_con_st *prev;
  drizzle_query_st *query;
  drizzle_result_st *result;
  drizzle_result_st *result_list;
  uint8_t *scramble;
  union
  {
    drizzle_con_tcp_st tcp;
    drizzle_con_uds_st uds;
  } socket;
  uint8_t buffer[DRIZZLE_MAX_BUFFER_SIZE];
  char db[DRIZZLE_MAX_DB_SIZE];
  char password[DRIZZLE_MAX_PASSWORD_SIZE];
  uint8_t scramble_buffer[DRIZZLE_MAX_SCRAMBLE_SIZE];
  char server_version[DRIZZLE_MAX_SERVER_VERSION_SIZE];
  drizzle_state_fn *state_stack[DRIZZLE_STATE_STACK_SIZE];
  char user[DRIZZLE_MAX_USER_SIZE];
};

/**
 * @ingroup drizzle_query
 */
struct drizzle_query_st
{
  drizzle_st *drizzle;
  drizzle_query_st *next;
  drizzle_query_st *prev;
  drizzle_query_options_t options;
  drizzle_query_state_t state;
  drizzle_con_st *con;
  drizzle_result_st *result;
  const char *string;
  size_t size;
  void *context;
  drizzle_query_context_free_fn *context_free_fn;
};

/**
 * @ingroup drizzle_result
 */
struct drizzle_result_st
{
  drizzle_con_st *con;
  drizzle_result_st *next;
  drizzle_result_st *prev;
  drizzle_result_options_t options;

  char info[DRIZZLE_MAX_INFO_SIZE];
  uint16_t error_code;
  char sqlstate[DRIZZLE_MAX_SQLSTATE_SIZE + 1];
  uint64_t insert_id;
  uint16_t warning_count;
  uint64_t affected_rows;

  uint16_t column_count;
  uint16_t column_current;
  drizzle_column_st *column_list;
  drizzle_column_st *column;
  drizzle_column_st *column_buffer;

  uint64_t row_count;
  uint64_t row_current;

  uint16_t field_current;
  size_t field_total;
  size_t field_offset;
  size_t field_size;
  drizzle_field_t field;
  drizzle_field_t field_buffer;

  uint64_t row_list_size;
  drizzle_row_t row;
  drizzle_row_t *row_list;
  size_t *field_sizes;
  size_t **field_sizes_list;
};

/**
 * @ingroup drizzle_column
 */
struct drizzle_column_st
{
  drizzle_result_st *result;
  drizzle_column_st *next;
  drizzle_column_st *prev;
  drizzle_column_options_t options;
  char catalog[DRIZZLE_MAX_CATALOG_SIZE];
  char db[DRIZZLE_MAX_DB_SIZE];
  char table[DRIZZLE_MAX_TABLE_SIZE];
  char orig_table[DRIZZLE_MAX_TABLE_SIZE];
  char name[DRIZZLE_MAX_COLUMN_NAME_SIZE];
  char orig_name[DRIZZLE_MAX_COLUMN_NAME_SIZE];
  drizzle_charset_t charset;
  uint32_t size;
  size_t max_size;
  drizzle_column_type_t type;
  drizzle_column_flags_t flags;
  uint8_t decimals;
  uint8_t default_value[DRIZZLE_MAX_DEFAULT_VALUE_SIZE];
  size_t default_value_size;
};

#ifdef __cplusplus
}
#endif

#endif /* __DRIZZLE_STRUCTS_H */
