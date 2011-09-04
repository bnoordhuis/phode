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
 * @brief drizzle_result_st tests
 */

#include "tests/common.h"

int main(void)
{
  drizzle_st *drizzle;
  drizzle_con_st *con;
  drizzle_result_st *result;
  drizzle_result_st result_buffer;
  drizzle_result_st *clone;

  printf("sizeof(drizzle_result_st) = %zu\n", sizeof(drizzle_result_st));

  if ((drizzle= drizzle_create(NULL)) == NULL)
    drizzle_test_error("drizzle_create");

  if ((con= drizzle_con_create(drizzle, NULL)) == NULL)
    drizzle_test_error("drizzle_con_create");

  if ((result= drizzle_result_create(con, &result_buffer)) == NULL)
    drizzle_test_error("drizzle_result_create");
  drizzle_result_free(result);

  if ((result= drizzle_result_create(con, NULL)) == NULL)
    drizzle_test_error("drizzle_result_create");

  if ((clone= drizzle_result_clone(con, NULL, result)) == NULL)
    drizzle_test_error("drizzle_result_clone");
  drizzle_result_free(clone);

  drizzle_result_set_info(result, "simple test");

  if (strcmp(drizzle_result_info(result), "simple test"))
    drizzle_test_error("drizzle_result_info");

  drizzle_result_free(result);
  drizzle_con_free(con);
  drizzle_free(drizzle);

  return 0;
}
