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
#include <unistd.h>

#include <libdrizzle/drizzle_client.h>
#include <libdrizzle/drizzle_server.h>

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

static void proxy(drizzle_st *drizzle, drizzle_con_st *server,
                  drizzle_con_st *client, drizzle_result_st *server_result,
                  drizzle_result_st *client_result, drizzle_column_st *column);

int main(int argc, char *argv[])
{
  int c;
  uint32_t count= 0;
  const char *server_host= NULL;
  const char *client_host= NULL;
  bool server_mysql= false;
  bool client_mysql= false;
  in_port_t server_port= 0;
  in_port_t client_port= 0;
  drizzle_verbose_t verbose= DRIZZLE_VERBOSE_NEVER;
  drizzle_return_t ret;
  drizzle_st drizzle;
  drizzle_con_st con_listen;
  drizzle_con_st server;
  drizzle_con_st client;
  drizzle_result_st server_result;
  drizzle_result_st client_result;
  drizzle_column_st column;

  while((c = getopt(argc, argv, "c:h:H:mMp:P:v")) != -1)
  {
    switch(c)
    {
    case 'c':
      count= (uint32_t)atoi(optarg);
      break;

    case 'h':
      server_host= optarg;
      break;

    case 'H':
      client_host= optarg;
      break;

    case 'm':
      server_mysql= true;
      break;

    case 'M':
      client_mysql= true;
      break;

    case 'p':
      server_port= (in_port_t)atoi(optarg);
      break;

    case 'P':
      client_port= (in_port_t)atoi(optarg);
      break;

    case 'v':
      verbose++;
      break;

    default:
      printf("\nusage: %s [-c <count>] [-h <host>] [-H <host>] [-m] [-M] "
             "[-p <port>] [-p <port>] [-v]\n", argv[0]);
      printf("\t-c <count> - Number of connections to accept before exiting\n");
      printf("\t-h <host>  - Host to listen on\n");
      printf("\t-H <host>  - Host to connect to\n");
      printf("\t-m         - Use MySQL protocol for incoming connections\n");
      printf("\t-M         - Use MySQL protocol for outgoing connectionsn\n");
      printf("\t-p <port>  - Port to listen on\n");
      printf("\t-P <port>  - Port to connect to\n");
      printf("\t-v         - Increase verbosity level\n");
      return 1;
    }
  }

  if (drizzle_create(&drizzle) == NULL)
  {
    printf("drizzle_create:NULL\n");
    return 1;
  }

  drizzle_add_options(&drizzle, DRIZZLE_FREE_OBJECTS);
  drizzle_set_verbose(&drizzle, verbose);

  if (drizzle_con_create(&drizzle, &con_listen) == NULL)
  {
    printf("drizzle_con_create:NULL\n");
    return 1;
  }

  drizzle_con_add_options(&con_listen, DRIZZLE_CON_LISTEN);
  drizzle_con_set_tcp(&con_listen, server_host, server_port);

  if (server_mysql)
    drizzle_con_add_options(&con_listen, DRIZZLE_CON_MYSQL);

  if (drizzle_con_listen(&con_listen) != DRIZZLE_RETURN_OK)
  {
    printf("drizzle_con_listen:%s\n", drizzle_error(&drizzle));
    return 1;
  }

  while (1)
  {
    (void)drizzle_con_accept(&drizzle, &server, &ret);
    if (ret != DRIZZLE_RETURN_OK)
    {
      printf("drizzle_con_accept:%s\n", drizzle_error(&drizzle));
      return 1;
    }

    if (drizzle_con_create(&drizzle, &client) == NULL)
    {
      printf("drizzle_con_create:NULL\n");
      return 1;
    }

    drizzle_con_add_options(&client,
                            DRIZZLE_CON_RAW_PACKET | DRIZZLE_CON_RAW_SCRAMBLE);
    if (client_mysql)
      drizzle_con_add_options(&client, DRIZZLE_CON_MYSQL);
    drizzle_con_set_tcp(&client, client_host, client_port);

    ret= drizzle_con_connect(&client);
    if (ret != DRIZZLE_RETURN_OK)
    {
      printf("drizzle_con_connect:%s\n", drizzle_error(&drizzle));
      return 1;
    }

    proxy(&drizzle, &server, &client, &server_result, &client_result, &column);

    drizzle_con_free(&client);
    drizzle_con_free(&server);

    if (count > 0)
    {
      count--;

      if (count == 0)
        break;
    }
  }

  drizzle_con_free(&con_listen);
  drizzle_free(&drizzle);

  return 0;
}

static void proxy(drizzle_st *drizzle, drizzle_con_st *server,
                  drizzle_con_st *client, drizzle_result_st *server_result,
                  drizzle_result_st *client_result, drizzle_column_st *column)
{
  drizzle_return_t ret;
  drizzle_command_t command;
  const uint8_t *data;
  size_t offset;
  size_t size;
  size_t total;
  uint64_t row;
  bool row_break;
  drizzle_field_t field;

  /* Handshake packets. */
  ret= drizzle_handshake_server_read(client);
  DRIZZLE_RETURN_CHECK(ret, "drizzle_handshake_server_read", drizzle)

  drizzle_con_copy_handshake(server, client);

  ret= drizzle_handshake_server_write(server);
  DRIZZLE_RETURN_CHECK(ret, "drizzle_handshake_server_write", drizzle)

  ret= drizzle_handshake_client_read(server);
  DRIZZLE_RETURN_CHECK(ret, "drizzle_handshake_client_read", drizzle)

  drizzle_con_copy_handshake(client, server);

  ret= drizzle_handshake_client_write(client);
  DRIZZLE_RETURN_CHECK(ret, "drizzle_handshake_client_write", drizzle)

  (void)drizzle_result_read(client, client_result, &ret);
  DRIZZLE_RETURN_CHECK(ret, "drizzle_result_read", drizzle)

  drizzle_con_set_status(server, drizzle_con_status(client));

  if (drizzle_result_clone(server, server_result, client_result) == NULL)
    DRIZZLE_RETURN_ERROR("drizzle_result_clone", drizzle)

  ret= drizzle_result_write(server, server_result, true);
  DRIZZLE_RETURN_CHECK(ret, "drizzle_result_write", drizzle)

  if (drizzle_result_error_code(client_result) != 0 ||
      drizzle_result_eof(client_result))
  {
    /* There was a handshake or authentication error. */
    return;
  }

  drizzle_con_add_options(client, DRIZZLE_CON_READY);

  /* Command loop. */
  while (1)
  {
    drizzle_result_free(server_result);
    drizzle_result_free(client_result);

    while (1)
    {
      data= drizzle_con_command_read(server, &command, &offset, &size, &total,
                                     &ret);
      if (ret == DRIZZLE_RETURN_LOST_CONNECTION)
        return;

      DRIZZLE_RETURN_CHECK(ret, "drizzle_con_command_read", drizzle)

      (void)drizzle_con_command_write(client, NULL, command, data, size, total,
                                      &ret);
      DRIZZLE_RETURN_CHECK(ret, "drizzle_con_command_write", drizzle)

      if ((offset + size) == total)
        break;
    }

    if (command == DRIZZLE_COMMAND_QUIT)
      return;
    else if (command == DRIZZLE_COMMAND_FIELD_LIST)
    {
      if (drizzle_result_create(client, client_result) == NULL)
        DRIZZLE_RETURN_ERROR("drizzle_result_create", drizzle)

      if (drizzle_result_create(server, server_result) == NULL)
        DRIZZLE_RETURN_ERROR("drizzle_result_create", drizzle)
    }
    else
    {
      (void)drizzle_result_read(client, client_result, &ret);
      DRIZZLE_RETURN_CHECK(ret, "drizzle_result_read", drizzle)

      drizzle_con_set_status(server, drizzle_con_status(client));
      if (drizzle_result_clone(server, server_result, client_result) == NULL)
        DRIZZLE_RETURN_ERROR("drizzle_result_clone", drizzle)

      if (drizzle_result_column_count(client_result) == 0)
      {
        /* Simple result with no column, row, or field data. */
        ret= drizzle_result_write(server, server_result, true);
        DRIZZLE_RETURN_CHECK(ret, "drizzle_result_write", drizzle)

        continue;
      }

      ret= drizzle_result_write(server, server_result, false);
      DRIZZLE_RETURN_CHECK(ret, "drizzle_result_write", drizzle)
    }

    /* Columns. */
    while (1)
    {
      if (drizzle_column_read(client_result, column, &ret) == NULL)
      {
        DRIZZLE_RETURN_CHECK(ret, "drizzle_column_read", drizzle)
        break;
      }

      DRIZZLE_RETURN_CHECK(ret, "drizzle_column_read", drizzle)

      ret= drizzle_column_write(server_result, column);
      DRIZZLE_RETURN_CHECK(ret, "drizzle_column_write", drizzle)

      drizzle_column_free(column);
    }

    drizzle_con_set_status(server, drizzle_con_status(client));
    drizzle_result_set_eof(server_result, true);

    if (command == DRIZZLE_COMMAND_FIELD_LIST)
    {
      ret= drizzle_result_write(server, server_result, true);
      DRIZZLE_RETURN_CHECK(ret, "drizzle_result_write", drizzle)
      continue;
    }
    else
    {
      ret= drizzle_result_write(server, server_result, false);
      DRIZZLE_RETURN_CHECK(ret, "drizzle_result_write", drizzle)
    }

    /* Rows. */
    while (1)
    {
      row= drizzle_row_read(client_result, &ret);
      DRIZZLE_RETURN_CHECK(ret, "drizzle_row_read", drizzle)

      if (row == 0)
        break;

      drizzle_result_set_row_size(server_result,
                                  drizzle_result_row_size(client_result));

      ret= drizzle_row_write(server_result);
      DRIZZLE_RETURN_CHECK(ret, "drizzle_row_write", drizzle)

      /* Fields. */
      row_break= false;
      while (row_break == false)
      {
        field= drizzle_field_read(client_result, &offset, &size, &total, &ret);
        if (ret == DRIZZLE_RETURN_ROW_END)
          break;
        else if (ret == DRIZZLE_RETURN_ROW_BREAK)
        {
          if (size == 0)
            break;

          row_break= true;
        }
        else
          DRIZZLE_RETURN_CHECK(ret, "drizzle_field_read", drizzle)

        ret= drizzle_field_write(server_result, field, size, total);
        DRIZZLE_RETURN_CHECK(ret, "drizzle_field_write", drizzle)
      }
    }

    ret= drizzle_result_write(server, server_result, true);
    DRIZZLE_RETURN_CHECK(ret, "drizzle_result_write", drizzle)
  }
}
