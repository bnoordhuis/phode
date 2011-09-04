/*
 * Drizzle Client & Protocol Library
 *
 * Copyright (C) 2008 Eric Day (eday@oddments.org)
 * All rights reserved.
 *
 * Use and distribution licensed under the BSD license.  See
 * the COPYING file in this directory for full text.
 */

#include "config.h"

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <libdrizzle/drizzle_client.h>

typedef enum
{
  CLIENT_QUERY,
  CLIENT_FIELDS,
  CLIENT_ROWS
} client_state;

typedef enum
{
  BUFFER_NONE,
  BUFFER_FIELD,
  BUFFER_ROW,
  BUFFER_ALL
} buffer_level;

typedef struct
{
  drizzle_con_st con;
  drizzle_result_st result;
  drizzle_column_st column;
  client_state state;
  uint64_t row;
} client_con_st;

typedef struct
{
  drizzle_st drizzle;
  bool mysql_protocol;
  client_con_st *client_con_list;
  uint32_t client_con_count;
  buffer_level level;
  char *query;
  size_t query_len;
} client_st;

char client_process(client_st *client, client_con_st *client_con);
void con_info(drizzle_con_st *con);
void result_info(drizzle_result_st *result);
void column_info(drizzle_column_st *column);

#define CLIENT_ERROR(__function, __ret, __client) { \
    printf(__function ":%d:%s\n", __ret, \
           drizzle_error(&((__client)->drizzle))); \
    exit(1); }

int main(int argc, char *argv[])
{
  client_st client;
  int c;
  char blocking= 0;
  drizzle_return_t ret;
  uint32_t x;
  int wait_for_connections= 0;
  drizzle_con_st *con;
  client_con_st *client_con;
  char *host= NULL;
  in_port_t port= 0;
  char *user= NULL;
  char *password= NULL;
  char *db= NULL;

  memset(&client, 0, sizeof(client_st));

  /* Use one connection by default. */
  client.client_con_count= 1;

  while ((c = getopt(argc, argv, "bB:c:d:h:Hmp:P:u:")) != -1)
  {
    switch(c)
    {
    case 'b':
      blocking= 1;
      break;

    case 'B':
      if (!strcasecmp(optarg, "none"))
        client.level= BUFFER_NONE;
      else if (!strcasecmp(optarg, "field"))
        client.level= BUFFER_FIELD;
      else if (!strcasecmp(optarg, "row"))
        client.level= BUFFER_ROW;
      else if (!strcasecmp(optarg, "all"))
        client.level= BUFFER_ALL;
      else
      {
        printf("Invalid buffer level: %s\n", optarg);
        exit(0);
      }
      break;

    case 'c':
      client.client_con_count= (uint32_t)atoi(optarg);
      break;

    case 'd':
      db= optarg;
      break;

    case 'h':
      host= optarg;
      break;

    case 'm':
      client.mysql_protocol= true;
      break;

    case 'p':
      password= optarg;
      break;

    case 'P':
      port= (in_port_t)atoi(optarg);
      break;

    case 'u':
      user= optarg;
      break;

    case 'H':
    default:
      printf("\nUsage: %s [options] [query]\n", argv[0]);
      printf("\t-b            - Use blocking sockets\n");
      printf("\t-B <level>    - Use buffer <level>, options are:\n");
      printf("\t                none - Don't buffer anything (default)\n");
      printf("\t                field - Only buffer individual fields\n");
      printf("\t                row - Only buffer individual rows\n");
      printf("\t                all - Buffer entire result\n");
      printf("\t-c <cons>     - Create <cons> connections\n");
      printf("\t-d <db>       - Use <db> for the connection\n");
      printf("\t-h <host>     - Connect to <host>\n");
      printf("\t-H            - Print this help menu\n");
      printf("\t-m            - Use MySQL protocol\n");
      printf("\t-p <password> - Use <password> for authentication\n");
      printf("\t-P <port>     - Connect to <port>\n");
      printf("\t-u <user>     - Use <user> for authentication\n");
      exit(0);
    }
  }

  if (argc != optind)
  {
    client.query= argv[optind];
    client.query_len= strlen(client.query);
  }

  if (client.client_con_count > 0)
  {
    client.client_con_list= calloc(client.client_con_count,
                                   sizeof(client_con_st));
    if (client.client_con_list == NULL)
    {
      printf("calloc:%d\n", errno);
      exit(1);
    }
  }

  /* This may fail if there is other initialization that fails. See docs. */
  if (drizzle_create(&(client.drizzle)) == NULL)
  {
    printf("drizzle_create failed\n");
    exit(1);
  }

  if (blocking == 0)
    drizzle_add_options(&(client.drizzle), DRIZZLE_NON_BLOCKING);

  /* Start all connections, and if in non-blocking mode, return as soon as the
     connection would block. In blocking mode, this completes the entire
     connection/query/result. */
  for (x= 0; x < client.client_con_count; x++)
  {
    /* This may fail if there is other initialization that fails. See docs. */
    con= drizzle_con_add_tcp(&(client.drizzle),
                              &(client.client_con_list[x].con),
                              host, port, user, password, db,
                              client.mysql_protocol ? DRIZZLE_CON_MYSQL : 0);
    if (con == NULL)
      CLIENT_ERROR("drizzle_con_add_tcp", 0, &client);
    drizzle_con_set_context(&(client.client_con_list[x].con),
                            &(client.client_con_list[x]));

    if (client_process(&client, &(client.client_con_list[x])) == 1)
      wait_for_connections++;
  }

  /* If in non-blocking mode, continue to process connections as they become
     ready. Loop exits when all connections have completed. */
  while (wait_for_connections != 0)
  {
    ret= drizzle_con_wait(&(client.drizzle));
    if (ret != DRIZZLE_RETURN_OK)
      CLIENT_ERROR("drizzle_con_wait", ret, &client);

    while ((con= drizzle_con_ready(&(client.drizzle))) != NULL)
    {
      client_con= (client_con_st *)drizzle_con_context(con);

      if (client_process(&client, client_con) == 0)
        wait_for_connections--;
    }
  }

  for (x= 0; x < client.client_con_count; x++)
    drizzle_con_free(&(client.client_con_list[x].con));

  drizzle_free(&(client.drizzle));

  if (client.client_con_list != NULL)
    free(client.client_con_list);

  return 0;
}

char client_process(client_st *client, client_con_st *client_con)
{
  drizzle_return_t ret;
  drizzle_column_st *column;
  size_t offset= 0;
  size_t length;
  size_t total;
  drizzle_field_t field;
  drizzle_row_t row;
  size_t *field_sizes;
  uint16_t x;

  switch (client_con->state)
  {
  case CLIENT_QUERY:
    if (client->query == NULL)
      break;

    /* This may fail if some allocation fails, but it will set ret. */
    (void)drizzle_query(&(client_con->con), &(client_con->result),
                        client->query, client->query_len, &ret);
    if (ret == DRIZZLE_RETURN_IO_WAIT)
      return 1;
    else if (ret != DRIZZLE_RETURN_OK)
      CLIENT_ERROR("drizzle_query", ret, client);

    result_info(&(client_con->result));

    if (drizzle_result_column_count(&(client_con->result)) == 0)
      break;

    client_con->state= CLIENT_FIELDS;

  case CLIENT_FIELDS:
    if (client->level == BUFFER_ALL)
    {
      ret= drizzle_result_buffer(&(client_con->result));
      if (ret == DRIZZLE_RETURN_IO_WAIT)
        return 1;
      else if (ret != DRIZZLE_RETURN_OK)
        CLIENT_ERROR("drizzle_result_buffer", ret, client);

      while ((column= drizzle_column_next(&(client_con->result))) != NULL)
        column_info(column);
    }
    else
    {
      while (1)
      {
        column= drizzle_column_read(&(client_con->result),
                                    &(client_con->column), &ret);
        if (ret == DRIZZLE_RETURN_IO_WAIT)
          return 1;
        else if (ret != DRIZZLE_RETURN_OK)
          CLIENT_ERROR("drizzle_column_read", ret, client);

        if (column == NULL)
          break;

        column_info(column);
        drizzle_column_free(column);
      }
    }

    client_con->state= CLIENT_ROWS;

  case CLIENT_ROWS:
    if (client->level == BUFFER_ALL)
    {
      /* Everything has been buffered, just loop through and print. */
      while ((row= drizzle_row_next(&(client_con->result))) != NULL)
      {
        field_sizes= drizzle_row_field_sizes(&(client_con->result));

        printf("Row: %" PRId64 "\n",
               drizzle_row_current(&(client_con->result)));

        for (x= 0; x < drizzle_result_column_count(&(client_con->result)); x++)
        {
          if (row[x] == NULL)
            printf("     (NULL)\n");
          else
          {
            printf("     (%zd) %.*s\n", field_sizes[x], (int32_t)field_sizes[x],
                   row[x]);
          }
        }

        printf("\n");
      }

      drizzle_result_free(&(client_con->result));
      break;
    }

    while (1)
    {
      if (client->level == BUFFER_NONE || client->level == BUFFER_FIELD)
      {
        /* Still need to read a row at a time, and then each field. */
        if (client_con->row == 0)
        {
          client_con->row= drizzle_row_read(&(client_con->result), &ret);
          if (ret == DRIZZLE_RETURN_IO_WAIT)
          {
            client_con->row= 0;
            return 1;
          }
          else if (ret != DRIZZLE_RETURN_OK)
            CLIENT_ERROR("drizzle_row", ret, client);

          if (client_con->row == 0)
          {
            drizzle_result_free(&(client_con->result));
            break;
          }

          printf("Row: %" PRId64 "\n", client_con->row);
        }

        while (1)
        {
          if (client->level == BUFFER_FIELD)
          {
            /* Since an entire field is buffered, we don't need to worry about
               partial reads. */
            field= drizzle_field_buffer(&(client_con->result), &total, &ret);
            length= total;
          }
          else
          {
            field= drizzle_field_read(&(client_con->result), &offset, &length,
                                      &total, &ret);
          }

          if (ret == DRIZZLE_RETURN_IO_WAIT)
            return 1;
          else if (ret == DRIZZLE_RETURN_ROW_END)
            break;
          else if (ret != DRIZZLE_RETURN_OK)
            CLIENT_ERROR("drizzle_field_read", ret, client);

          if (field == NULL)
            printf("     (NULL)");
          else if (offset > 0)
            printf("%.*s", (int32_t)length, field);
          else
            printf("     (%zd) %.*s", total, (int32_t)length, field);

          if (offset + length == total)
            printf("\n");

          /* If we buffered the entire field, be sure to free it. */
          if (client->level == BUFFER_FIELD)
            drizzle_field_free(field);
        }

        client_con->row= 0;
        printf("\n");
      }
      else if (client->level == BUFFER_ROW)
      {
        /* The entire row will be buffered here, so no need to worry about
           partial reads. */
        row = drizzle_row_buffer(&(client_con->result), &ret);
        if (ret == DRIZZLE_RETURN_IO_WAIT)
          return 1;
        else if (ret != DRIZZLE_RETURN_OK)
          CLIENT_ERROR("drizzle_row", ret, client);

        /* This marks the end of rows. */
        if (row == NULL)
          break;

        field_sizes= drizzle_row_field_sizes(&(client_con->result));

        printf("Row: %" PRId64 "\n",
               drizzle_row_current(&(client_con->result)));

        for (x= 0; x < drizzle_result_column_count(&(client_con->result)); x++)
        {
          if (row[x] == NULL)
            printf("     (NULL)\n");
          else
          {
            printf("     (%zd) %.*s\n", field_sizes[x], (int32_t)field_sizes[x],
                   row[x]);
          }
        }

        drizzle_row_free(&(client_con->result), row);
        printf("\n");
      }
    }

    drizzle_result_free(&(client_con->result));
    break;

  default:
    /* This should be impossible. */
    return 1;
  }

  return 0;
}

void con_info(drizzle_con_st *con)
{
  printf("Connected: protocol_version=%u\n"
         "                    version=%s\n"
         "                  thread_id=%u\n"
         "               capabilities=%u\n"
         "                   language=%u\n"
         "                     status=%u\n\n",
         drizzle_con_protocol_version(con), drizzle_con_server_version(con),
         drizzle_con_thread_id(con), drizzle_con_capabilities(con),
         drizzle_con_charset(con), drizzle_con_status(con));
}

void result_info(drizzle_result_st *result)
{
  printf("Result:     row_count=%" PRId64 "\n"
         "            insert_id=%" PRId64 "\n"
         "        warning_count=%u\n"
         "         column_count=%u\n"
         "        affected_rows=%" PRId64 "\n\n",
         drizzle_result_row_count(result),
         drizzle_result_insert_id(result),
         drizzle_result_warning_count(result),
         drizzle_result_column_count(result),
         drizzle_result_affected_rows(result));
}

void column_info(drizzle_column_st *column)
{
  printf("Field:   catalog=%s\n"
         "              db=%s\n"
         "           table=%s\n"
         "       org_table=%s\n"
         "            name=%s\n"
         "        org_name=%s\n"
         "         charset=%u\n"
         "            size=%u\n"
         "        max_size=%zu\n"
         "            type=%u\n"
         "           flags=%u\n\n",
         drizzle_column_catalog(column), drizzle_column_db(column),
         drizzle_column_table(column), drizzle_column_orig_table(column),
         drizzle_column_name(column), drizzle_column_orig_name(column),
         drizzle_column_charset(column), drizzle_column_size(column),
         drizzle_column_max_size(column), drizzle_column_type(column),
         drizzle_column_flags(column));
}
