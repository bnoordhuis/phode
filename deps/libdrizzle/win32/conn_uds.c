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
 * @brief UDS Connection stubs for Windows
 */

#include "libdrizzle/common.h"
#include "libdrizzle/conn_private.h"

/*
 * Common definitions
 */

const char *drizzle_con_uds(drizzle_con_st *con)
{
  (void)con;
  return (const char *)NULL;
}

void drizzle_con_set_uds(drizzle_con_st *con, const char *uds)
{
  (void)con;
  (void)uds;
}

/*
 * Private definitions
 */

bool drizzle_con_uses_uds(drizzle_con_st *con)
{
  (void)con;
  return false;
}

void drizzle_con_clone_uds(drizzle_con_st *con, drizzle_con_st *from)
{
  (void)con;
  (void)from;
}
