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
#include <string.h>

#include <libdrizzle/drizzle_client.h>

#define SIMPLE_MULTI_COUNT 10

int main(int argc, char *argv[])
{
  const char *query= "SELECT table_schema,table_name FROM tables";
  drizzle_st drizzle;
  drizzle_con_st con[SIMPLE_MULTI_COUNT];
  drizzle_result_st result[SIMPLE_MULTI_COUNT];
  drizzle_query_st ql[SIMPLE_MULTI_COUNT];
  drizzle_return_t ret;
  drizzle_row_t row;
  int x;

  if (drizzle_create(&drizzle) == NULL)
  {
    printf("drizzle_create:NULL\n");
    return 1;
  }

  /* Create SIMPLE_MULTI_COUNT connections and initialize query list. */
  for (x= 0; x < SIMPLE_MULTI_COUNT; x++)
  {
    if (x == 0)
    {
      if (drizzle_con_create(&drizzle, &(con[0])) == NULL)
      {
        printf("drizzle_con_create:%s\n", drizzle_error(&drizzle));
        return 1;
      }

      if (argc == 2 && !strcmp(argv[1], "-m"))
        drizzle_con_add_options(&(con[0]), DRIZZLE_CON_MYSQL);
      else if (argc != 1)
      {
        printf("usage: %s [-m]\n", argv[0]);
        return 1;
      }

      drizzle_con_set_db(&(con[0]), "information_schema");
    }
    else
    {
      if (drizzle_con_clone(&drizzle, &(con[x]), &(con[0])) == NULL)
      {
        printf("drizzle_con_clone:%s\n", drizzle_error(&drizzle));
        return 1;
      }
    }

    if (drizzle_query_add(&drizzle, &(ql[x]), &(con[x]), &(result[x]), query,
                          strlen(query), 0, NULL) == NULL)
    {
      printf("drizzle_query_add:%s\n", drizzle_error(&drizzle));
      return 1;
    }
  }

  ret= drizzle_query_run_all(&drizzle);
  if (ret != DRIZZLE_RETURN_OK)
  {
    printf("drizzle_query_run_all:%s\n", drizzle_error(&drizzle));
    return 1;
  }

  for (x= 0; x < SIMPLE_MULTI_COUNT; x++)
  {
    if (drizzle_result_error_code(&(result[x])) != 0)
    {
      printf("%d:%s\n", drizzle_result_error_code(&(result[x])),
             drizzle_result_error(&(result[x])));
      continue;
    }

    while ((row= drizzle_row_next(&(result[x]))) != NULL)
      printf("%d %s:%s\n", x, row[0], row[1]);
  }

  drizzle_free(&drizzle);

  return 0;
}
