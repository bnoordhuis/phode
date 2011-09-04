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
 * @brief Includes and macros for tests
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libdrizzle/drizzle_client.h>
#include <libdrizzle/drizzle_server.h>

#define drizzle_test(...) do \
{ \
  printf(__VA_ARGS__); \
  printf("\n"); \
} while (0);

#define drizzle_test_error(...) do \
{ \
  printf("*** %s:%d *** ", __FILE__, __LINE__); \
  printf(__VA_ARGS__); \
  printf("\n"); \
  exit(1); \
} while (0);
