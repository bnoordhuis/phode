/*
 * Drizzle Client & Protocol Library
 *
 * Copyright (C) 2008 Eric Day (eday@oddments.org)
 * All rights reserved.
 *
 * Use and distribution licensed under the BSD license.  See
 * the COPYING file in this directory for full text.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <libdrizzle/drizzle_client.h>

int main(int argc, char *argv[])
{
  int c;
  const char *db= "INFORMATION_SCHEMA";
  const char *host= NULL;
  bool mysql= false;
  in_port_t port= 0;
  const char *query= "SELECT TABLE_SCHEMA,TABLE_NAME FROM TABLES";
  drizzle_verbose_t verbose= DRIZZLE_VERBOSE_NEVER;
  drizzle_st drizzle;
  drizzle_con_st con;
  drizzle_result_st result;
  drizzle_return_t ret;
  int x;
  char **row;

  while ((c = getopt(argc, argv, "d:h:mp:q:v")) != -1)
  {
    switch(c)
    {
    case 'd':
      db= optarg;
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

    case 'q':
      query= optarg;
      break;

    case 'v':
      verbose++;
      break;

    default:
      printf("usage: %s [-d <db>] [-h <host>] [-m] [-p <port>] [-q <query>] "
             "[-v]\n", argv[0]);
      printf("\t-d <db>    - Database to use for query\n");
      printf("\t-h <host>  - Host to listen on\n");
      printf("\t-m         - Use the MySQL protocol\n");
      printf("\t-p <port>  - Port to listen on\n");
      printf("\t-q <query> - Query to run\n");
      printf("\t-v         - Increase verbosity level\n");
      return 1;
    }
  }

  if (drizzle_create(&drizzle) == NULL)
  {
    printf("drizzle_create:NULL\n");
    return 1;
  }

  drizzle_set_verbose(&drizzle, verbose);

  if (drizzle_con_create(&drizzle, &con) == NULL)
  {
    printf("drizzle_con_create:NULL\n");
    return 1;
  }

  if (mysql)
    drizzle_con_add_options(&con, DRIZZLE_CON_MYSQL);

  drizzle_con_set_tcp(&con, host, port);
  drizzle_con_set_db(&con, db);

  (void)drizzle_query_str(&con, &result, query, &ret);
  if (ret != DRIZZLE_RETURN_OK)
  {
    printf("drizzle_query:%s\n", drizzle_con_error(&con));
    return 1;
  }

  ret= drizzle_result_buffer(&result);
  if (ret != DRIZZLE_RETURN_OK)
  {
    printf("drizzle_result_buffer:%s\n", drizzle_con_error(&con));
    return 1;
  }

  while ((row= (char **)drizzle_row_next(&result)) != NULL)
  {
    for (x= 0; x < drizzle_result_column_count(&result); x++)
      printf("%s%s", x == 0 ? "" : ":", row[x] == NULL ? "NULL" : row[x]);
    printf("\n");
  }

  drizzle_result_free(&result);
  drizzle_con_free(&con);
  drizzle_free(&drizzle);

  return 0;
}
