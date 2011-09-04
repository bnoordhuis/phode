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
 * @brief Connection Definitions for Unix Domain Sockets
 */

#include "common.h"

const char *drizzle_con_uds(const drizzle_con_st *con)
{
  if (con->socket_type == DRIZZLE_CON_SOCKET_UDS)
  {
    if (con->socket.uds.sockaddr.sun_path[0] != 0)
      return con->socket.uds.sockaddr.sun_path;

    if (con->options & DRIZZLE_CON_MYSQL)
      return DRIZZLE_DEFAULT_UDS_MYSQL;

    return DRIZZLE_DEFAULT_UDS;
  }

  return NULL;
}

void drizzle_con_set_uds(drizzle_con_st *con, const char *uds)
{
  drizzle_con_reset_addrinfo(con);

  con->socket_type= DRIZZLE_CON_SOCKET_UDS;

  if (uds == NULL)
    uds= "";

  con->socket.uds.sockaddr.sun_family= AF_UNIX;
  strncpy(con->socket.uds.sockaddr.sun_path, uds,
          sizeof(con->socket.uds.sockaddr.sun_path));
  con->socket.uds.sockaddr.sun_path[sizeof(con->socket.uds.sockaddr.sun_path) - 1]= 0;

  con->socket.uds.addrinfo.ai_family= AF_UNIX;
  con->socket.uds.addrinfo.ai_socktype= SOCK_STREAM;
  con->socket.uds.addrinfo.ai_protocol= 0;
  con->socket.uds.addrinfo.ai_addrlen= sizeof(struct sockaddr_un);
  con->socket.uds.addrinfo.ai_addr= (struct sockaddr *)&(con->socket.uds.sockaddr);
}
