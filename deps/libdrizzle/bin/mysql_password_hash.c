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

#include <stdio.h>
#include <string.h>

#include <libdrizzle/drizzle_client.h>

#define BUFFER_CHUNK 8192

int main(int argc, char *argv[])
{
  char hashed_password[DRIZZLE_MYSQL_PASSWORD_HASH];

  if (argc != 2)
  {
    printf("Usage: %s <password to hash>\n", argv[0]);
    return 1;
  }

  drizzle_mysql_password_hash(hashed_password, argv[1], strlen(argv[1]));

  printf("%s\n", hashed_password);

  return 0;
}
