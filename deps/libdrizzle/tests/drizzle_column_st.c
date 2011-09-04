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
 * @brief drizzle_column_st tests
 */

#include "tests/common.h"

int main(void)
{
  drizzle_st *drizzle;
  drizzle_con_st *con;
  drizzle_result_st *result;
  drizzle_column_st *column;
  drizzle_column_st column_buffer;

  printf("sizeof(drizzle_column_st) = %zu\n", sizeof(drizzle_column_st));

  drizzle_test("drizzle_create");
  if ((drizzle= drizzle_create(NULL)) == NULL)
    drizzle_test_error("returned NULL");

  drizzle_test("drizzle_con_create");
  if ((con= drizzle_con_create(drizzle, NULL)) == NULL)
    drizzle_test_error("returned NULL");

  drizzle_test("drizzle_result_create");
  if ((result= drizzle_result_create(con, NULL)) == NULL)
    drizzle_test_error("returned NULL");

  drizzle_test("drizzle_column_create buffer");
  if ((column= drizzle_column_create(result, &column_buffer)) == NULL)
    drizzle_test_error("returned NULL");
  drizzle_column_free(column);

  drizzle_test("drizzle_column_create");
  if ((column= drizzle_column_create(result, NULL)) == NULL)
    drizzle_test_error("returned NULL");

  drizzle_test("drizzle_column_set_catalog");
  drizzle_column_set_catalog(column, "simple test");

  drizzle_test("drizzle_column_catalog");
  if (strcmp(drizzle_column_catalog(column), "simple test"))
    drizzle_test_error("does not match what was set");

  drizzle_test("drizzle_column_free");
  drizzle_column_free(column);

  drizzle_test("drizzle_result_free");
  drizzle_result_free(result);

  drizzle_test("drizzle_con_free");
  drizzle_con_free(con);

  drizzle_test("drizzle_free");
  drizzle_free(drizzle);

  return 0;
}
