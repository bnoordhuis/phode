/*
 * Drizzle Client & Protocol Library
 *
 * Copyright (C) 2008 Eric Day (eday@oddments.org)
 * All rights reserved.
 *
 * Use and distribution licensed under the BSD license.  See
 * the COPYING file in this directory for full text.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include <libdrizzle/drizzle_server.h>
#include <sqlite3.h>

#define SQLITE_SERVER_VERSION "SQLite Server using libdrizzle 0.1"

#define DRIZZLE_RETURN_CHECK(__ret, __function, __drizzle) \
{ \
  if ((__ret) != DRIZZLE_RETURN_OK) \
    DRIZZLE_RETURN_ERROR(__function, __drizzle) \
}

#define DRIZZLE_RETURN_ERROR(__function, __drizzle) \
{ \
  printf(__function ":%s\n", drizzle_error(__drizzle)); \
  return; \
}

#define DRIZZLE_RETURN_CHECK_VAL(__ret, __function, __drizzle) \
{ \
  if ((__ret) != DRIZZLE_RETURN_OK) \
  { \
    printf(__function ":%s\n", drizzle_error(__drizzle)); \
    return ret; \
  } \
}

typedef struct
{
  drizzle_st drizzle;
  drizzle_con_st con;
  drizzle_result_st result;
  drizzle_column_st column;
  sqlite3* db;
  bool send_columns;
  drizzle_verbose_t verbose;
  uint64_t rows;
} sqlite_server;

static void server_run(sqlite_server *server);
static int row_cb(void *data, int field_count, char **fields, char **columns);
static drizzle_return_t send_version(sqlite_server *server);
static void usage(char *name);

int main(int argc, char *argv[])
{
  int c;
  uint32_t count= 0;
  const char *host= NULL;
  bool mysql= false;
  in_port_t port= 0;
  drizzle_return_t ret;
  sqlite_server server;
  drizzle_con_st con_listen;

  server.db= NULL;
  server.verbose= DRIZZLE_VERBOSE_NEVER;

  while((c = getopt(argc, argv, "c:h:mp:v")) != -1)
  {
    switch(c)
    {
    case 'c':
      count= (uint32_t)atoi(optarg);
      break;

    case 'h':
      host= optarg;
      break;

    case 'm':
      mysql= true;
      break;

    case 'p':
      port= (in_port_t)atoi(optarg);
      break;

    case 'v':
      server.verbose++;
      break;

    default:
      usage(argv[0]);
      return 1;
    }
  }

  if (argc != (optind + 1))
  {
    usage(argv[0]);
    return 1;
  }

  sqlite3_open(argv[optind], &(server.db));
  if (server.db == NULL)
  {
    printf("sqlite3_open: could not open sqlite3 db\n");
    return 1;
  }

  if (drizzle_create(&server.drizzle) == NULL)
  {
    printf("drizzle_create:NULL\n");
    return 1;
  }

  drizzle_add_options(&server.drizzle, DRIZZLE_FREE_OBJECTS);
  drizzle_set_verbose(&server.drizzle, server.verbose);

  if (drizzle_con_create(&server.drizzle, &con_listen) == NULL)
  {
    printf("drizzle_con_create:NULL\n");
    return 1;
  }

  drizzle_con_add_options(&con_listen, DRIZZLE_CON_LISTEN);
  drizzle_con_set_tcp(&con_listen, host, port);

  if (mysql)
    drizzle_con_add_options(&con_listen, DRIZZLE_CON_MYSQL);

  if (drizzle_con_listen(&con_listen) != DRIZZLE_RETURN_OK)
  {
    printf("drizzle_con_listen:%s\n", drizzle_error(&server.drizzle));
    return 1;
  }

  while (1)
  {
    (void)drizzle_con_accept(&server.drizzle, &server.con, &ret);
    if (ret != DRIZZLE_RETURN_OK)
    {
      printf("drizzle_con_accept:%s\n", drizzle_error(&server.drizzle));
      return 1;
    }

    server_run(&server);

    drizzle_con_free(&server.con);

    if (count > 0)
    {
      count--;

      if (count == 0)
        break;
    }
  }

  drizzle_con_free(&con_listen);
  drizzle_free(&server.drizzle);
  sqlite3_close(server.db);

  return 0;
}

static void server_run(sqlite_server *server)
{
  drizzle_return_t ret;
  drizzle_command_t command;
  uint8_t *data= NULL;
  size_t total;
  int sqlite_ret;
  char *sqlite_err;

  /* Handshake packets. */
  drizzle_con_set_protocol_version(&(server->con), 10);
  drizzle_con_set_server_version(&(server->con), "libdrizzle+SQLite");
  drizzle_con_set_thread_id(&(server->con), 1);
  drizzle_con_set_scramble(&(server->con),
                           (const uint8_t *)"ABCDEFGHIJKLMNOPQRST");
  drizzle_con_set_capabilities(&(server->con), DRIZZLE_CAPABILITIES_NONE);
  drizzle_con_set_charset(&(server->con), 8);
  drizzle_con_set_status(&(server->con), DRIZZLE_CON_STATUS_NONE);
  drizzle_con_set_max_packet_size(&(server->con), DRIZZLE_MAX_PACKET_SIZE);

  ret= drizzle_handshake_server_write(&(server->con));
  DRIZZLE_RETURN_CHECK(ret, "drizzle_handshake_server_write",
                       &(server->drizzle))

  ret= drizzle_handshake_client_read(&(server->con));
  DRIZZLE_RETURN_CHECK(ret, "drizzle_handshake_client_read", &(server->drizzle))

  if (drizzle_result_create(&(server->con), &(server->result)) == NULL)
    DRIZZLE_RETURN_ERROR("drizzle_result_create", &(server->drizzle))

  ret= drizzle_result_write(&(server->con), &(server->result), true);
  DRIZZLE_RETURN_CHECK(ret, "drizzle_result_write", &(server->drizzle))

  /* Command loop. */
  while (1)
  {
    drizzle_result_free(&(server->result));
    if (data != NULL)
      free(data);

    data= drizzle_con_command_buffer(&(server->con), &command, &total, &ret);
    if (ret == DRIZZLE_RETURN_LOST_CONNECTION ||
        (ret == DRIZZLE_RETURN_OK && command == DRIZZLE_COMMAND_QUIT))
    {
      if (data != NULL)
        free(data);
      return;
    }
    DRIZZLE_RETURN_CHECK(ret, "drizzle_con_command_buffer", &(server->drizzle))

    if (server->verbose >= DRIZZLE_VERBOSE_INFO)
    {
      printf("Command=%u Data=%s\n", command,
             data == NULL ? "NULL" : (char *)data);
    }

    if (drizzle_result_create(&(server->con), &(server->result)) == NULL)
      DRIZZLE_RETURN_ERROR("drizzle_result_create", &(server->drizzle))

    if (command != DRIZZLE_COMMAND_QUERY ||
        !strcasecmp((char *)data, "SHOW DATABASES"))
    {
      ret= drizzle_result_write(&(server->con), &(server->result), true);
      DRIZZLE_RETURN_CHECK(ret, "drizzle_result_write", &(server->drizzle))

      if (command == DRIZZLE_COMMAND_FIELD_LIST)
      {
        drizzle_result_set_eof(&(server->result), true);
        ret= drizzle_result_write(&(server->con), &(server->result), true);
        DRIZZLE_RETURN_CHECK(ret, "drizzle_result_write", &(server->drizzle))
      }

      continue;
    }

    if (strstr((char *)data, "@@version") != NULL)
    {
      ret= send_version(server);
      if (ret != DRIZZLE_RETURN_OK)
        return;

      continue;
    }

    server->send_columns= true;
    server->rows= 0;

    if (!strcasecmp((char *)data, "SHOW TABLES"))
    {
      sqlite_ret= sqlite3_exec(server->db,
                            "SELECT name FROM sqlite_master WHERE type='table'",
                               row_cb, server, &sqlite_err);
    }
    else
    {
      sqlite_ret= sqlite3_exec(server->db, (char *)data, row_cb, server,
                               &sqlite_err);
    }

    if (sqlite_ret != SQLITE_OK)
    {
      if (sqlite_err == NULL)
        printf("sqlite3_exec failed\n");
      else
      {
        drizzle_result_set_error_code(&(server->result), (uint16_t)sqlite_ret);
        drizzle_result_set_error(&(server->result), sqlite_err);
        ret= drizzle_result_write(&(server->con), &(server->result), true);
        DRIZZLE_RETURN_CHECK(ret, "drizzle_result_write", &(server->drizzle))

        printf("sqlite3_exec:%s\n", sqlite_err);
        sqlite3_free(sqlite_err);
      }

      return;
    }

    if (server->rows == 0)
    {
      drizzle_result_set_column_count(&(server->result), 0);
      ret= drizzle_result_write(&(server->con), &(server->result), true);
      DRIZZLE_RETURN_CHECK(ret, "drizzle_result_write", &(server->drizzle))
    }
    else
    {
      drizzle_result_set_eof(&(server->result), true);
      ret= drizzle_result_write(&(server->con), &(server->result), true);
      DRIZZLE_RETURN_CHECK(ret, "drizzle_result_write", &(server->drizzle))
    }
  }
}

static int row_cb(void *data, int field_count, char **fields, char **columns)
{
  sqlite_server *server= (sqlite_server *)data;
  drizzle_return_t ret;
  int x;
  size_t sizes[8192];

  if (server->send_columns == true)
  {
    server->send_columns= false;
    drizzle_result_set_column_count(&(server->result), (uint16_t)field_count);

    ret= drizzle_result_write(&(server->con), &(server->result), false);
    DRIZZLE_RETURN_CHECK_VAL(ret, "drizzle_result_write", &(server->drizzle))

    if (drizzle_column_create(&(server->result), &(server->column)) == NULL)
    {
      DRIZZLE_RETURN_CHECK_VAL(DRIZZLE_RETURN_MEMORY, "drizzle_column_create",
                               &(server->drizzle))
    }

    drizzle_column_set_catalog(&(server->column), "sqlite");
    drizzle_column_set_db(&(server->column), "sqlite_db");
    drizzle_column_set_table(&(server->column), "sqlite_table");
    drizzle_column_set_orig_table(&(server->column), "sqlite_table");
    drizzle_column_set_charset(&(server->column), 8);
    drizzle_column_set_type(&(server->column), DRIZZLE_COLUMN_TYPE_VARCHAR);

    for (x= 0; x < field_count; x++)
    {
      drizzle_column_set_size(&(server->column),
                              fields[x] == NULL ?
                              0 : (uint32_t)strlen(fields[x]));
      drizzle_column_set_name(&(server->column), columns[x]);
      drizzle_column_set_orig_name(&(server->column), columns[x]);

      ret= drizzle_column_write(&(server->result), &(server->column));
      DRIZZLE_RETURN_CHECK_VAL(ret, "drizzle_column_write", &(server->drizzle))
    }

    drizzle_column_free(&(server->column));

    drizzle_result_set_eof(&(server->result), true);

    ret= drizzle_result_write(&(server->con), &(server->result), false);
    DRIZZLE_RETURN_CHECK_VAL(ret, "drizzle_result_write", &(server->drizzle))
  }

  for (x= 0; x < field_count; x++)
  {
    if (fields[x] == NULL)
      sizes[x]= 0;
    else
      sizes[x]= strlen(fields[x]);
  }

  /* This is needed for MySQL and old Drizzle protocol. */
  drizzle_result_calc_row_size(&(server->result), (drizzle_field_t *)fields,
                               sizes);

  ret= drizzle_row_write(&(server->result));
  DRIZZLE_RETURN_CHECK_VAL(ret, "drizzle_row_write", &(server->drizzle))

  for (x= 0; x < field_count; x++)
  {
    ret= drizzle_field_write(&(server->result), (drizzle_field_t)fields[x],
                             sizes[x], sizes[x]);
    DRIZZLE_RETURN_CHECK_VAL(ret, "drizzle_field_write", &(server->drizzle))
  }

  server->rows++;

  return 0;
}

static drizzle_return_t send_version(sqlite_server *server)
{
  drizzle_return_t ret;
  drizzle_field_t fields[1];
  size_t sizes[1];

  fields[0]= (drizzle_field_t)SQLITE_SERVER_VERSION;
  sizes[0]= strlen(SQLITE_SERVER_VERSION);

  drizzle_result_set_column_count(&(server->result), 1);

  ret= drizzle_result_write(&(server->con), &(server->result), false);
  DRIZZLE_RETURN_CHECK_VAL(ret, "drizzle_result_write", &(server->drizzle))

  if (drizzle_column_create(&(server->result), &(server->column)) == NULL)
  {
    DRIZZLE_RETURN_CHECK_VAL(DRIZZLE_RETURN_MEMORY, "drizzle_column_create",
                             &(server->drizzle))
  }

  drizzle_column_set_catalog(&(server->column), "sqlite");
  drizzle_column_set_db(&(server->column), "sqlite_db");
  drizzle_column_set_table(&(server->column), "sqlite_table");
  drizzle_column_set_orig_table(&(server->column), "sqlite_table");
  drizzle_column_set_charset(&(server->column), 8);
  drizzle_column_set_type(&(server->column), DRIZZLE_COLUMN_TYPE_VARCHAR);
  drizzle_column_set_size(&(server->column), (uint32_t)sizes[0]);
  drizzle_column_set_name(&(server->column), "version");
  drizzle_column_set_orig_name(&(server->column), "version");

  ret= drizzle_column_write(&(server->result), &(server->column));
  DRIZZLE_RETURN_CHECK_VAL(ret, "drizzle_column_write", &(server->drizzle))

  drizzle_column_free(&(server->column));

  drizzle_result_set_eof(&(server->result), true);

  ret= drizzle_result_write(&(server->con), &(server->result), false);
  DRIZZLE_RETURN_CHECK_VAL(ret, "drizzle_result_write", &(server->drizzle))

  /* This is needed for MySQL and old Drizzle protocol. */
  drizzle_result_calc_row_size(&(server->result), fields, sizes);

  ret= drizzle_row_write(&(server->result));
  DRIZZLE_RETURN_CHECK_VAL(ret, "drizzle_row_write", &(server->drizzle))

  ret= drizzle_field_write(&(server->result), fields[0], sizes[0], sizes[0]);
  DRIZZLE_RETURN_CHECK_VAL(ret, "drizzle_field_write", &(server->drizzle))

  ret= drizzle_result_write(&(server->con), &(server->result), true);
  DRIZZLE_RETURN_CHECK_VAL(ret, "drizzle_result_write", &(server->drizzle))

  return DRIZZLE_RETURN_OK;
}

static void usage(char *name)
{
  printf("\nusage: %s [-c <count>] [-h <host>] [-m] [-p <port>] [-v] "
         "<sqlite3 db file>\n", name);
  printf("\t-c <count> - Number of connections to accept before exiting\n");
  printf("\t-h <host>  - Host to listen on\n");
  printf("\t-m         - Use the MySQL protocol\n");
  printf("\t-p <port>  - Port to listen on\n");
  printf("\t-v         - Increase verbosity level\n");
}
