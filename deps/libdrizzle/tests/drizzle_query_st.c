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
 * @brief drizzle_query_st tests
 */

#include "tests/common.h"

int main(void)
{
  drizzle_st *drizzle;
  drizzle_query_st *query;
  drizzle_query_st query_buffer;
  size_t size;

  printf("sizeof(drizzle_query_st) = %zu\n", sizeof(drizzle_query_st));

  if ((drizzle= drizzle_create(NULL)) == NULL)
    drizzle_test_error("drizzle_create");

  if ((query= drizzle_query_create(drizzle, &query_buffer)) == NULL)
    drizzle_test_error("drizzle_query_create");
  drizzle_query_free(query);

  if ((query= drizzle_query_create(drizzle, NULL)) == NULL)
    drizzle_test_error("drizzle_query_create");

  if (drizzle_query_options(query) != DRIZZLE_QUERY_ALLOCATED)
    drizzle_test_error("drizzle_query_options");

  drizzle_query_set_string(query, "SELECT 1+1", 10);

  if (strncmp(drizzle_query_string(query, &size), "SELECT 1+1", 10) ||
      size != 10)
  {
    drizzle_test_error("drizzle_query_string");
  }

  drizzle_query_free(query);
  drizzle_free(drizzle);

  return 0;
}
