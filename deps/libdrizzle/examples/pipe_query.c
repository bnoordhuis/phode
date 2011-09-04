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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <libdrizzle/drizzle_client.h>

#define BUFFER_CHUNK 8192

int main(int argc, char *argv[])
{
  int c;
  char *host= NULL;
  in_port_t port= 0;
  char *user= NULL;
  char *password= NULL;
  char *buffer= NULL;
  size_t buffer_size= 0;
  size_t buffer_total= 0;
  ssize_t read_size= 0;
  drizzle_st drizzle;
  drizzle_con_st con;
  drizzle_result_st result;
  drizzle_return_t ret;
  drizzle_field_t field;
  size_t offset;
  size_t size;
  size_t total;

  /* The docs say this might fail, so check for errors. */
  if (drizzle_create(&drizzle) == NULL)
  {
    printf("drizzle_create:failed\n");
    exit(1);
  }

  if (drizzle_con_create(&drizzle, &con) == NULL)
  {
    printf("drizzle_con_create:%s\n", drizzle_error(&drizzle));
    exit(1);
  }

  while ((c = getopt(argc, argv, "d:h:Hmp:P:u:")) != -1)
  {
    switch(c)
    {
    case 'd':
      drizzle_con_set_db(&con, optarg);
      break;

    case 'h':
      host= optarg;
      break;

    case 'm':
      drizzle_con_add_options(&con, DRIZZLE_CON_MYSQL);
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

  drizzle_con_set_tcp(&con, host, port);
  drizzle_con_set_auth(&con, user, password);

  do
  {
    if (read_size == -1)
    {
      printf("read:%d\n", errno);
      return 1;
    }

    buffer_size+= (size_t)read_size;

    buffer= realloc(buffer, buffer_size + BUFFER_CHUNK);
    if (buffer == NULL)
    {
      printf("realloc:%d\n", errno);
      return 1;
    }

    buffer_total= buffer_size + BUFFER_CHUNK;
  } while ((read_size= read(0, buffer + buffer_size, BUFFER_CHUNK)) != 0);

  (void)drizzle_query(&con, &result, buffer, buffer_size, &ret);
  if (ret != DRIZZLE_RETURN_OK)
  {
    printf("drizzle_query:%s\n", drizzle_error(&drizzle));
    return 1;
  }

  free(buffer);

  ret= drizzle_column_skip(&result);
  if (ret != DRIZZLE_RETURN_OK)
  {
    printf("drizzle_column_skip:%s\n", drizzle_error(&drizzle));
    return 1;
  }

  while (drizzle_row_read(&result, &ret) != 0 && ret == DRIZZLE_RETURN_OK)
  {
    while (1)
    {
      field= drizzle_field_read(&result, &offset, &size, &total, &ret);
      if (ret == DRIZZLE_RETURN_ROW_END)
        break;
      else if (ret != DRIZZLE_RETURN_OK)
      {
        printf("drizzle_field_read:%s\n", drizzle_error(&drizzle));
        return 1;
      }

      if (field == NULL)
        printf("NULL");
      else
        printf("%.*s", (int)size, field);

      if (offset + size == total)
        printf("\t");
    }

    printf("\n");
  }

  if (ret != DRIZZLE_RETURN_OK)
  {
    printf("drizzle_row_read:%s\n", drizzle_error(&drizzle));
    return 1;
  }

  drizzle_result_free(&result);
  drizzle_con_free(&con);
  drizzle_free(&drizzle);

  return 0;
}
