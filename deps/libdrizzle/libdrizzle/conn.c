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
 * @brief Connection Definitions
 */

#include "common.h"

/**
 * @addtogroup drizzle_con_static Static Connection Declarations
 * @ingroup drizzle_con
 * @{
 */

/**
 * Set socket options for a connection.
 *
 * @param[in] con Connection structure previously initialized with
 *  drizzle_con_create(), drizzle_con_clone(), or related functions.
 * @return Standard drizzle return value.
 */
static drizzle_return_t _con_setsockopt(drizzle_con_st *con);

/** @} */

/*
 * Common Definitions
 */

int drizzle_con_fd(const drizzle_con_st *con)
{
  return con->fd;
}

drizzle_return_t drizzle_con_set_fd(drizzle_con_st *con, int fd)
{
  drizzle_return_t ret;

  con->fd= fd;

  ret= _con_setsockopt(con);
  if (ret != DRIZZLE_RETURN_OK)
    con->drizzle->last_errno= errno;

  return ret;
}

void drizzle_con_close(drizzle_con_st *con)
{
  if (con->fd == -1)
    return;

  (void)close(con->fd);
  con->fd= -1;

  con->options&= (drizzle_con_options_t)~DRIZZLE_CON_READY;
  con->packet_number= 0;
  con->buffer_ptr= con->buffer;
  con->buffer_size= 0;
  con->events= 0;
  con->revents= 0;

  drizzle_state_reset(con);
}

drizzle_return_t drizzle_con_set_events(drizzle_con_st *con, short events)
{
  drizzle_return_t ret;

  if ((con->events | events) == con->events)
    return DRIZZLE_RETURN_OK;

  con->events|= events;

  if (con->drizzle->event_watch_fn != NULL)
  {
    ret= con->drizzle->event_watch_fn(con, con->events,
                                      con->drizzle->event_watch_context);
    if (ret != DRIZZLE_RETURN_OK)
    {
      drizzle_con_close(con);
      return ret;
    }
  }

  return DRIZZLE_RETURN_OK;
}

drizzle_return_t drizzle_con_set_revents(drizzle_con_st *con, short revents)
{
  drizzle_return_t ret;

  if (revents != 0)
    con->options|= DRIZZLE_CON_IO_READY;

  con->revents= revents;

  /* Remove external POLLOUT watch if we didn't ask for it. Otherwise we spin
     forever until another POLLIN state change. This is much more efficient
     than removing POLLOUT on every state change since some external polling
     mechanisms need to use a system call to change flags (like Linux epoll). */
  if (revents & POLLOUT && !(con->events & POLLOUT) &&
      con->drizzle->event_watch_fn != NULL)
  {
    ret= con->drizzle->event_watch_fn(con, con->events,
                                      con->drizzle->event_watch_context);
    if (ret != DRIZZLE_RETURN_OK)
    {
      drizzle_con_close(con);
      return ret;
    }
  }

  con->events&= (short)~revents;

  return DRIZZLE_RETURN_OK;
}

drizzle_st *drizzle_con_drizzle(const drizzle_con_st *con)
{
  return con->drizzle;
}

const char *drizzle_con_error(const drizzle_con_st *con)
{
  return drizzle_error(con->drizzle);
}

int drizzle_con_errno(const drizzle_con_st *con)
{
  return drizzle_errno(con->drizzle);
}

uint16_t drizzle_con_error_code(const drizzle_con_st *con)
{
  return drizzle_error_code(con->drizzle);
}

const char *drizzle_con_sqlstate(const drizzle_con_st *con)
{
  return drizzle_sqlstate(con->drizzle);
}

drizzle_con_options_t drizzle_con_options(const drizzle_con_st *con)
{
  return con->options;
}

void drizzle_con_set_options(drizzle_con_st *con,
                             drizzle_con_options_t options)
{
  con->options= options;
}

void drizzle_con_add_options(drizzle_con_st *con,
                             drizzle_con_options_t options)
{
  con->options|= options;

  /* If asking for the experimental Drizzle protocol, clean the MySQL flag. */
  if (con->options & DRIZZLE_CON_EXPERIMENTAL)
    con->options&= (drizzle_con_options_t)~DRIZZLE_CON_MYSQL;
}

void drizzle_con_remove_options(drizzle_con_st *con,
                                drizzle_con_options_t options)
{
  con->options&= ~options;
}

const char *drizzle_con_host(const drizzle_con_st *con)
{
  if (con->socket_type == DRIZZLE_CON_SOCKET_TCP)
  {
    if (con->socket.tcp.host == NULL && !(con->options & DRIZZLE_CON_LISTEN))
      return DRIZZLE_DEFAULT_TCP_HOST;

    return con->socket.tcp.host;
  }

  return NULL;
}

in_port_t drizzle_con_port(const drizzle_con_st *con)
{
  if (con->socket_type == DRIZZLE_CON_SOCKET_TCP)
  {
    if (con->socket.tcp.port != 0)
      return con->socket.tcp.port;

    if (con->options & DRIZZLE_CON_MYSQL)
      return DRIZZLE_DEFAULT_TCP_PORT_MYSQL;

    return DRIZZLE_DEFAULT_TCP_PORT;
  }

  return 0;
}

void drizzle_con_set_tcp(drizzle_con_st *con, const char *host, in_port_t port)
{
  drizzle_con_reset_addrinfo(con);

  con->socket_type= DRIZZLE_CON_SOCKET_TCP;

  if (host == NULL)
    con->socket.tcp.host= NULL;
  else
  {
    con->socket.tcp.host= con->socket.tcp.host_buffer;
    strncpy(con->socket.tcp.host, host, NI_MAXHOST);
    con->socket.tcp.host[NI_MAXHOST - 1]= 0;
  }

  con->socket.tcp.port= port;
}

const char *drizzle_con_user(const drizzle_con_st *con)
{
  return con->user;
}

const char *drizzle_con_password(const drizzle_con_st *con)
{
  return con->password;
}

void drizzle_con_set_auth(drizzle_con_st *con, const char *user,
                          const char *password)
{
  if (user == NULL)
    con->user[0]= 0;
  else
  {
    strncpy(con->user, user, DRIZZLE_MAX_USER_SIZE);
    con->user[DRIZZLE_MAX_USER_SIZE - 1]= 0;
  }

  if (password == NULL)
    con->password[0]= 0;
  else
  {
    strncpy(con->password, password, DRIZZLE_MAX_PASSWORD_SIZE);
    con->password[DRIZZLE_MAX_PASSWORD_SIZE - 1]= 0;
  }
}

const char *drizzle_con_db(const drizzle_con_st *con)
{
  return con->db;
}

void drizzle_con_set_db(drizzle_con_st *con, const char *db)
{
  if (db == NULL)
    con->db[0]= 0;
  else
  {
    strncpy(con->db, db, DRIZZLE_MAX_DB_SIZE);
    con->db[DRIZZLE_MAX_DB_SIZE - 1]= 0;
  }
}

void *drizzle_con_context(const drizzle_con_st *con)
{
  return con->context;
}

void drizzle_con_set_context(drizzle_con_st *con, void *context)
{
  con->context= context;
}

void drizzle_con_set_context_free_fn(drizzle_con_st *con,
                                     drizzle_con_context_free_fn *function)
{
  con->context_free_fn= function;
}

uint8_t drizzle_con_protocol_version(const drizzle_con_st *con)
{
  return con->protocol_version;
}

const char *drizzle_con_server_version(const drizzle_con_st *con)
{
  return con->server_version;
}

uint32_t drizzle_con_server_version_number(const drizzle_con_st *con)
{
  uint32_t major;
  uint32_t minor;
  uint32_t version;
  const char *current;
  char *end;

  current= con->server_version;

  major= (uint32_t)strtoul(current, &end, 10);
  current= end + 1;
  minor= (uint32_t)strtoul(current, &end, 10);
  current= end + 1;
  version= (uint32_t)strtoul(current, &end, 10);

  return (major * 10000) + (minor * 100) + version;
}

uint32_t drizzle_con_thread_id(const drizzle_con_st *con)
{
  return con->thread_id;
}

const uint8_t *drizzle_con_scramble(const drizzle_con_st *con)
{
  return con->scramble;
}

drizzle_capabilities_t drizzle_con_capabilities(const drizzle_con_st *con)
{
  return con->capabilities;
}

drizzle_charset_t drizzle_con_charset(const drizzle_con_st *con)
{
  return con->charset;
}

drizzle_con_status_t drizzle_con_status(const drizzle_con_st *con)
{
  return con->status;
}

uint32_t drizzle_con_max_packet_size(const drizzle_con_st *con)
{
  return con->max_packet_size;
}

/*
 * Client Definitions
 */

drizzle_return_t drizzle_con_connect(drizzle_con_st *con)
{
  if (con->options & DRIZZLE_CON_READY)
    return DRIZZLE_RETURN_OK;

  if (drizzle_state_none(con))
  {
    if (!(con->options & DRIZZLE_CON_RAW_PACKET))
    {
      drizzle_state_push(con, drizzle_state_handshake_server_read);
      drizzle_state_push(con, drizzle_state_packet_read);
    }

    drizzle_state_push(con, drizzle_state_connect);
    drizzle_state_push(con, drizzle_state_addrinfo);
  }

  return drizzle_state_loop(con);
}

drizzle_result_st *drizzle_con_quit(drizzle_con_st *con,
                                    drizzle_result_st *result,
                                    drizzle_return_t *ret_ptr)
{
  return drizzle_con_command_write(con, result, DRIZZLE_COMMAND_QUIT, NULL, 0,
                                   0, ret_ptr);
}

drizzle_result_st *drizzle_quit(drizzle_con_st *con,
                                drizzle_result_st *result,
                                drizzle_return_t *ret_ptr)
{
  return drizzle_con_quit(con, result, ret_ptr);
}

drizzle_result_st *drizzle_con_select_db(drizzle_con_st *con,
                                         drizzle_result_st *result,
                                         const char *db,
                                         drizzle_return_t *ret_ptr)
{
  drizzle_con_set_db(con, db);
  return drizzle_con_command_write(con, result, DRIZZLE_COMMAND_INIT_DB,
                                   db, strlen(db), strlen(db), ret_ptr);
}

drizzle_result_st *drizzle_select_db(drizzle_con_st *con,
                                     drizzle_result_st *result,
                                     const char *db,
                                     drizzle_return_t *ret_ptr)
{
  return drizzle_con_select_db(con, result, db, ret_ptr);
}

drizzle_result_st *drizzle_con_shutdown(drizzle_con_st *con,
                                        drizzle_result_st *result,
                                        drizzle_return_t *ret_ptr)
{
  if (con->options & DRIZZLE_CON_MYSQL)
  {
    return drizzle_con_command_write(con, result, DRIZZLE_COMMAND_SHUTDOWN,
                                     "0", 1, 1, ret_ptr);
  }

  return drizzle_con_command_write(con, result, DRIZZLE_COMMAND_SHUTDOWN, NULL,
                                   0, 0, ret_ptr);
}

drizzle_result_st *drizzle_shutdown(drizzle_con_st *con,
                                    drizzle_result_st *result, uint32_t level,
                                    drizzle_return_t *ret_ptr)
{
  (void) level;
  return drizzle_con_shutdown(con, result, ret_ptr);
}

drizzle_result_st *drizzle_con_ping(drizzle_con_st *con,
                                    drizzle_result_st *result,
                                    drizzle_return_t *ret_ptr)
{
  return drizzle_con_command_write(con, result, DRIZZLE_COMMAND_PING, NULL, 0,
                                   0, ret_ptr);
}

drizzle_result_st *drizzle_ping(drizzle_con_st *con,
                                drizzle_result_st *result,
                                drizzle_return_t *ret_ptr)
{
  return drizzle_con_ping(con, result, ret_ptr);
}

drizzle_result_st *drizzle_con_command_write(drizzle_con_st *con,
                                             drizzle_result_st *result,
                                             drizzle_command_t command,
                                             const void *data, size_t size,
                                             size_t total,
                                             drizzle_return_t *ret_ptr)
{
  if (!(con->options & DRIZZLE_CON_READY))
  {
    if (con->options & DRIZZLE_CON_RAW_PACKET)
    {
      drizzle_set_error(con->drizzle, "drizzle_command_write",
                        "connection not ready");
      *ret_ptr= DRIZZLE_RETURN_NOT_READY;
      return result;
    }

    *ret_ptr= drizzle_con_connect(con);
    if (*ret_ptr != DRIZZLE_RETURN_OK)
      return result;
  }

  if (drizzle_state_none(con))
  {
    if (con->options & (DRIZZLE_CON_RAW_PACKET | DRIZZLE_CON_NO_RESULT_READ))
      con->result= NULL;
    else
    {
      con->result= drizzle_result_create(con, result);
      if (con->result == NULL)
      {
        *ret_ptr= DRIZZLE_RETURN_MEMORY;
        return NULL;
      }
    }

    con->command= command;
    con->command_data= (uint8_t *)data;
    con->command_size= size;
    con->command_offset= 0;
    con->command_total= total;

    drizzle_state_push(con, drizzle_state_command_write);
  }
  else if (con->command_data == NULL)
  {
    con->command_data= (uint8_t *)data;
    con->command_size= size;
  }

  *ret_ptr= drizzle_state_loop(con);
  if (*ret_ptr == DRIZZLE_RETURN_PAUSE)
    *ret_ptr= DRIZZLE_RETURN_OK;
  else if (*ret_ptr != DRIZZLE_RETURN_OK &&
           *ret_ptr != DRIZZLE_RETURN_IO_WAIT &&
           *ret_ptr != DRIZZLE_RETURN_ERROR_CODE)
  {
    drizzle_result_free(con->result);
    con->result= result;
  }

  return con->result;
}

/*
 * Server Definitions
 */

drizzle_return_t drizzle_con_listen(drizzle_con_st *con)
{
  if (con->options & DRIZZLE_CON_READY)
    return DRIZZLE_RETURN_OK;

  if (drizzle_state_none(con))
  {
    drizzle_state_push(con, drizzle_state_listen);
    drizzle_state_push(con, drizzle_state_addrinfo);
  }

  return drizzle_state_loop(con);
}

int drizzle_con_backlog(const drizzle_con_st *con)
{
  return con->backlog;
}

void drizzle_con_set_backlog(drizzle_con_st *con, int backlog)
{
  con->backlog= backlog;
}

void drizzle_con_set_protocol_version(drizzle_con_st *con,
                                      uint8_t protocol_version)
{
  con->protocol_version= protocol_version;
}

void drizzle_con_set_server_version(drizzle_con_st *con,
                                    const char *server_version)
{
  if (server_version == NULL)
    con->server_version[0]= 0;
  else
  {
    strncpy(con->server_version, server_version,
            DRIZZLE_MAX_SERVER_VERSION_SIZE);
    con->server_version[DRIZZLE_MAX_SERVER_VERSION_SIZE - 1]= 0;
  }
}

void drizzle_con_set_thread_id(drizzle_con_st *con, uint32_t thread_id)
{
  con->thread_id= thread_id;
}

void drizzle_con_set_scramble(drizzle_con_st *con, const uint8_t *scramble)
{
  if (scramble == NULL)
    con->scramble= NULL;
  else
  {
    con->scramble= con->scramble_buffer;
    memcpy(con->scramble, scramble, DRIZZLE_MAX_SCRAMBLE_SIZE);
  }
}

void drizzle_con_set_capabilities(drizzle_con_st *con,
                                  drizzle_capabilities_t capabilities)
{
  con->capabilities= capabilities;
}

void drizzle_con_set_charset(drizzle_con_st *con, drizzle_charset_t charset)
{
  con->charset= charset;
}

void drizzle_con_set_status(drizzle_con_st *con, drizzle_con_status_t status)
{
  con->status= status;
}

void drizzle_con_set_max_packet_size(drizzle_con_st *con,
                                     uint32_t max_packet_size)
{
  con->max_packet_size= max_packet_size;
}

void drizzle_con_copy_handshake(drizzle_con_st *con, drizzle_con_st *from)
{
  drizzle_con_set_auth(con, from->user, NULL);
  drizzle_con_set_scramble(con, from->scramble);
  drizzle_con_set_db(con, from->db);
  drizzle_con_set_protocol_version(con, from->protocol_version);
  drizzle_con_set_server_version(con, from->server_version);
  drizzle_con_set_thread_id(con, from->thread_id);
  drizzle_con_set_scramble(con, from->scramble);
  drizzle_con_set_capabilities(con, from->capabilities);
  drizzle_con_set_charset(con, from->charset);
  drizzle_con_set_status(con, from->status);
  drizzle_con_set_max_packet_size(con, from->max_packet_size);
}

void *drizzle_con_command_read(drizzle_con_st *con,
                               drizzle_command_t *command, size_t *offset,
                               size_t *size, size_t *total,
                               drizzle_return_t *ret_ptr)
{
  if (drizzle_state_none(con))
  {
    con->packet_number= 0;
    con->command_offset= 0;
    con->command_total= 0;

    drizzle_state_push(con, drizzle_state_command_read);
    drizzle_state_push(con, drizzle_state_packet_read);
  }

  *offset= con->command_offset;

  *ret_ptr= drizzle_state_loop(con);
  if (*ret_ptr == DRIZZLE_RETURN_PAUSE)
    *ret_ptr= DRIZZLE_RETURN_OK;

  *command= con->command;
  *size= con->command_size;
  *total= con->command_total;

  return con->command_data;
}

void *drizzle_con_command_buffer(drizzle_con_st *con,
                                 drizzle_command_t *command, size_t *total,
                                 drizzle_return_t *ret_ptr)
{
  uint8_t *command_data;
  size_t offset= 0;
  size_t size= 0;

  command_data= drizzle_con_command_read(con, command, &offset, &size, total,
                                         ret_ptr);
  if (*ret_ptr != DRIZZLE_RETURN_OK)
    return NULL;

  if (command_data == NULL)
  {
    *total= 0;
    return NULL;
  }

  if (con->command_buffer == NULL)
  {
    con->command_buffer= malloc((*total) + 1);
    if (con->command_buffer == NULL)
    {
      drizzle_set_error(con->drizzle, "drizzle_command_buffer", "malloc");
      *ret_ptr= DRIZZLE_RETURN_MEMORY;
      return NULL;
    }
  }

  memcpy(con->command_buffer + offset, command_data, size);

  while ((offset + size) != (*total))
  {
    command_data= drizzle_con_command_read(con, command, &offset, &size, total,
                                           ret_ptr);
    if (*ret_ptr != DRIZZLE_RETURN_OK)
      return NULL;

    memcpy(con->command_buffer + offset, command_data, size);
  }

  command_data= con->command_buffer;
  con->command_buffer= NULL;
  command_data[*total]= 0;

  return command_data;
}

/*
 * Local Definitions
 */

void drizzle_con_reset_addrinfo(drizzle_con_st *con)
{
  switch (con->socket_type)
  {
  case DRIZZLE_CON_SOCKET_TCP:
    if (con->socket.tcp.addrinfo != NULL)
    {
      freeaddrinfo(con->socket.tcp.addrinfo);
      con->socket.tcp.addrinfo= NULL;
    }
    break;

  case DRIZZLE_CON_SOCKET_UDS:
    con->socket.uds.addrinfo.ai_addr= NULL;
    break;

  default:
    break;
  }

  con->addrinfo_next= NULL;
}

/*
 * State Definitions
 */

drizzle_return_t drizzle_state_addrinfo(drizzle_con_st *con)
{
  drizzle_con_tcp_st *tcp;
  const char *host;
  char port[NI_MAXSERV];
  struct addrinfo ai;
  int ret;

  drizzle_log_debug(con->drizzle, "drizzle_state_addrinfo");

  switch (con->socket_type)
  {
  case DRIZZLE_CON_SOCKET_TCP:
    tcp= &(con->socket.tcp);

    if (tcp->addrinfo != NULL)
    {
      freeaddrinfo(tcp->addrinfo);
      tcp->addrinfo= NULL;
    }

    if (tcp->port != 0)
      snprintf(port, NI_MAXSERV, "%u", tcp->port);
    else if (con->options & DRIZZLE_CON_MYSQL)
      snprintf(port, NI_MAXSERV, "%u", DRIZZLE_DEFAULT_TCP_PORT_MYSQL);
    else
      snprintf(port, NI_MAXSERV, "%u", DRIZZLE_DEFAULT_TCP_PORT);

    memset(&ai, 0, sizeof(struct addrinfo));
    ai.ai_socktype= SOCK_STREAM;
    ai.ai_protocol= IPPROTO_TCP;

    if (con->options & DRIZZLE_CON_LISTEN)
    {
      ai.ai_flags = AI_PASSIVE;
      ai.ai_family = AF_UNSPEC;
      host= tcp->host;
    }
    else
    {
      if (tcp->host == NULL)
        host= DRIZZLE_DEFAULT_TCP_HOST;
      else
        host= tcp->host;
    }

    ret= getaddrinfo(host, port, &ai, &(tcp->addrinfo));
    if (ret != 0)
    {
      drizzle_set_error(con->drizzle, "drizzle_state_addrinfo",
                        "getaddrinfo:%s", gai_strerror(ret));
      return DRIZZLE_RETURN_GETADDRINFO;
    }

    con->addrinfo_next= tcp->addrinfo;

    break;

  case DRIZZLE_CON_SOCKET_UDS:
    con->addrinfo_next= &(con->socket.uds.addrinfo);
    break;

  default:
    break;
  }

  drizzle_state_pop(con);
  return DRIZZLE_RETURN_OK;
}

drizzle_return_t drizzle_state_connect(drizzle_con_st *con)
{
  int ret;
  drizzle_return_t dret;

  drizzle_log_debug(con->drizzle, "drizzle_state_connect");

  if (con->fd != -1)
  {
    (void)close(con->fd);
    con->fd= -1;
  }

  if (con->addrinfo_next == NULL)
  {
    drizzle_set_error(con->drizzle, "drizzle_state_connect",
                      "could not connect");
    drizzle_state_reset(con);
    return DRIZZLE_RETURN_COULD_NOT_CONNECT;
  }

  con->fd= socket(con->addrinfo_next->ai_family,
                  con->addrinfo_next->ai_socktype,
                  con->addrinfo_next->ai_protocol);
  if (con->fd == -1)
  {
    drizzle_set_error(con->drizzle, "drizzle_state_connect", "socket:%d",
                      errno);
    con->drizzle->last_errno= errno;
    return DRIZZLE_RETURN_ERRNO;
  }

  dret= _con_setsockopt(con);
  if (dret != DRIZZLE_RETURN_OK)
  {
    con->drizzle->last_errno= errno;
    return dret;
  }

  while (1)
  {
    ret= connect(con->fd, con->addrinfo_next->ai_addr,
                 con->addrinfo_next->ai_addrlen);

#ifdef _WIN32
    /*Mapping windows specific error codes to Posix*/
    errno = WSAGetLastError();
    switch(errno) {
    case WSAEINVAL:
    case WSAEALREADY:
    case WSAEWOULDBLOCK:
      errno = EINPROGRESS;
      break;
    default:
      break;
    }
#endif /* _WIN32 */
	
    drizzle_log_crazy(con->drizzle, "connect return=%d errno=%d", ret, errno);

    if (ret == 0)
    {
      con->addrinfo_next= NULL;
      break;
    }

    if (errno == EAGAIN || errno == EINTR)
      continue;

    if (errno == EINPROGRESS)
    {
      drizzle_state_pop(con);
      drizzle_state_push(con, drizzle_state_connecting);
      return DRIZZLE_RETURN_OK;
    }

    if (errno == ECONNREFUSED || errno == ENETUNREACH || errno == ETIMEDOUT)
    {
      con->addrinfo_next= con->addrinfo_next->ai_next;
      return DRIZZLE_RETURN_OK;
    }

    drizzle_set_error(con->drizzle, "drizzle_state_connect", "connect:%d",
                      errno);
    con->drizzle->last_errno= errno;
    return DRIZZLE_RETURN_ERRNO;
  }

  drizzle_state_pop(con);
  return DRIZZLE_RETURN_OK;
}

drizzle_return_t drizzle_state_connecting(drizzle_con_st *con)
{
  drizzle_return_t ret;

  drizzle_log_debug(con->drizzle, "drizzle_state_connecting");

  while (1)
  {
    if (con->revents & POLLOUT)
    {
      drizzle_state_pop(con);
      return DRIZZLE_RETURN_OK;
    }
    else if (con->revents & (POLLERR | POLLHUP | POLLNVAL))
    {
      con->revents= 0;
      drizzle_state_pop(con);
      drizzle_state_push(con, drizzle_state_connect);
      con->addrinfo_next= con->addrinfo_next->ai_next;
      return DRIZZLE_RETURN_OK;
    }

    ret= drizzle_con_set_events(con, POLLOUT);
    if (ret != DRIZZLE_RETURN_OK)
      return ret;

    if (con->drizzle->options & DRIZZLE_NON_BLOCKING)
      return DRIZZLE_RETURN_IO_WAIT;

    ret= drizzle_con_wait(con->drizzle);
    if (ret != DRIZZLE_RETURN_OK)
      return ret;
  }
}

drizzle_return_t drizzle_state_read(drizzle_con_st *con)
{
  drizzle_return_t ret;
  ssize_t read_size;

  drizzle_log_debug(con->drizzle, "drizzle_state_read");

  if (con->buffer_size == 0)
    con->buffer_ptr= con->buffer;
  else if ((con->buffer_ptr - con->buffer) > (DRIZZLE_MAX_BUFFER_SIZE / 2))
  {
    memmove(con->buffer, con->buffer_ptr, con->buffer_size);
    con->buffer_ptr= con->buffer;
  }

  while (1)
  {
    read_size = recv(con->fd, (char *)con->buffer_ptr + con->buffer_size,
                     (size_t)DRIZZLE_MAX_BUFFER_SIZE -
                     ((size_t)(con->buffer_ptr - con->buffer) +
                      con->buffer_size),0); 
#ifdef _WIN32
    /*Get windows error codes and map it to Posix*/
    errno = WSAGetLastError();
    switch(errno) {
    case WSAEWOULDBLOCK:
    case 10057:
      errno = EAGAIN;
      break;
    default:
      break;
    }
#endif /* _WIN32 */	
    drizzle_log_crazy(con->drizzle, "read fd=%d return=%zd errno=%d", con->fd,
                      read_size, errno);

    if (read_size == 0)
    {
      drizzle_set_error(con->drizzle, "drizzle_state_read",
                        "lost connection to server (EOF)");
      return DRIZZLE_RETURN_LOST_CONNECTION;
    }
    else if (read_size == -1)
    {
      if (errno == EAGAIN)
      {
        ret= drizzle_con_set_events(con, POLLIN);
        if (ret != DRIZZLE_RETURN_OK)
          return 0;

        if (con->drizzle->options & DRIZZLE_NON_BLOCKING)
          return DRIZZLE_RETURN_IO_WAIT;

        ret= drizzle_con_wait(con->drizzle);
        if (ret != DRIZZLE_RETURN_OK)
          return ret;

        continue;
      }
      else if (errno == ECONNREFUSED)
      {
        con->revents= 0;
        drizzle_state_pop(con);
        drizzle_state_push(con, drizzle_state_connect);
        con->addrinfo_next= con->addrinfo_next->ai_next;
        return DRIZZLE_RETURN_OK;
      }
      else if (errno == EINTR)
        continue;
      else if (errno == EPIPE || errno == ECONNRESET)
      {
        drizzle_set_error(con->drizzle, "drizzle_state_read",
                          "lost connection to server (%d)", errno);
        return DRIZZLE_RETURN_LOST_CONNECTION;
      }

      drizzle_set_error(con->drizzle, "drizzle_state_read", "read:%d", errno);
      con->drizzle->last_errno= errno;
      return DRIZZLE_RETURN_ERRNO;
    }

    con->buffer_size+= (size_t)read_size;
    break;
  }

  drizzle_state_pop(con);;
  return DRIZZLE_RETURN_OK;
}

drizzle_return_t drizzle_state_write(drizzle_con_st *con)
{
  drizzle_return_t ret;
  ssize_t write_size;

  drizzle_log_debug(con->drizzle, "drizzle_state_write");

  while (con->buffer_size != 0)
  {
  
    write_size = send(con->fd,(char *) con->buffer_ptr, con->buffer_size,0);

    drizzle_log_crazy(con->drizzle, "write fd=%d return=%zd errno=%d", con->fd,
                      write_size, errno);

    if (write_size == 0)
    {
      drizzle_set_error(con->drizzle, "drizzle_state_write",
                        "lost connection to server (EOF)");
      return DRIZZLE_RETURN_LOST_CONNECTION;
    }
    else if (write_size == -1)
    {
      if (errno == EAGAIN)
      {
        ret= drizzle_con_set_events(con, POLLOUT);
        if (ret != DRIZZLE_RETURN_OK)
          return ret;

        if (con->drizzle->options & DRIZZLE_NON_BLOCKING)
          return DRIZZLE_RETURN_IO_WAIT;

        ret= drizzle_con_wait(con->drizzle);
        if (ret != DRIZZLE_RETURN_OK)
          return ret;

        continue;
      }
      else if (errno == EINTR)
        continue;
      else if (errno == EPIPE || errno == ECONNRESET)
      {
        drizzle_set_error(con->drizzle, "drizzle_state_write",
                          "lost connection to server (%d)", errno);
        return DRIZZLE_RETURN_LOST_CONNECTION;
      }

      drizzle_set_error(con->drizzle, "drizzle_state_write", "write:%d", errno);
      con->drizzle->last_errno= errno;
      return DRIZZLE_RETURN_ERRNO;
    }

    con->buffer_ptr+= write_size;
    con->buffer_size-= (size_t)write_size;
    if (con->buffer_size == 0)
      break;
  }

  con->buffer_ptr= con->buffer;

  drizzle_state_pop(con);
  return DRIZZLE_RETURN_OK;
}

drizzle_return_t drizzle_state_listen(drizzle_con_st *con)
{
  char host[NI_MAXHOST];
  char port[NI_MAXSERV];
  int ret;
  int fd;
  int opt;
  drizzle_con_st *new_con;

  for (; con->addrinfo_next != NULL;
       con->addrinfo_next= con->addrinfo_next->ai_next)
  {
    ret= getnameinfo(con->addrinfo_next->ai_addr,
                     con->addrinfo_next->ai_addrlen, host, NI_MAXHOST, port,
                     NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
    if (ret != 0)
    {
      drizzle_set_error(con->drizzle, "drizzle_state_listen", "getnameinfo:%s",
                        gai_strerror(ret));
      return DRIZZLE_RETURN_GETADDRINFO;
    }

    /* Call to socket() can fail for some getaddrinfo results, try another. */
    fd= socket(con->addrinfo_next->ai_family, con->addrinfo_next->ai_socktype,
               con->addrinfo_next->ai_protocol);
    if (fd == -1)
    {
      drizzle_log_info(con->drizzle, "could not listen on %s:%s", host, port);
      drizzle_set_error(con->drizzle, "drizzle_state_listen", "socket:%d",
                        errno);
      continue;
    }
	
	opt= 1;
#ifdef _WIN32
        ret= setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,(const char*) &opt, sizeof(opt));
#else
        ret= setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif /* _WIN32 */
    if (ret == -1)
    {
      close(fd);
      drizzle_set_error(con->drizzle, "drizzle_state_listen", "setsockopt:%d",
                        errno);
      return DRIZZLE_RETURN_ERRNO;
    }

    ret= bind(fd, con->addrinfo_next->ai_addr, con->addrinfo_next->ai_addrlen);
    if (ret == -1)
    {
      close(fd);
      drizzle_set_error(con->drizzle, "drizzle_state_listen", "bind:%d", errno);
      if (errno == EADDRINUSE)
      {
        if (con->fd == -1)
        {
          drizzle_log_info(con->drizzle, "could not listen on %s:%s", host,
                           port);
        }

        continue;
      }

      return DRIZZLE_RETURN_ERRNO;
    }

    if (listen(fd, con->backlog) == -1)
    {
      close(fd);
      drizzle_set_error(con->drizzle, "drizzle_state_listen", "listen:%d",
                        errno);
      return DRIZZLE_RETURN_ERRNO;
    }

    if (con->fd == -1)
    {
      con->fd= fd;
      new_con= con;
    }
    else
    {
      new_con= drizzle_con_clone(con->drizzle, NULL, con);
      if (new_con == NULL)
      {
        close(fd);
        return DRIZZLE_RETURN_MEMORY;
      }

      new_con->fd= fd;
    }

    /* Wait for read events on the listening socket. */
    ret= drizzle_con_set_events(new_con, POLLIN);
    if (ret != DRIZZLE_RETURN_OK)
    {
      drizzle_con_free(new_con);
      return ret;
    }

    drizzle_log_info(con->drizzle, "listening on %s:%s", host, port);
  }

  /* Report last socket() error if we couldn't find an address to bind. */
  if (con->fd == -1)
    return DRIZZLE_RETURN_ERRNO;

  drizzle_state_pop(con);
  return DRIZZLE_RETURN_OK;
}

/*
 * Static Definitions
 */

static drizzle_return_t _con_setsockopt(drizzle_con_st *con)
{
  int ret;
  struct linger linger;
  struct timeval waittime;

  ret= 1;

#ifdef _WIN32
  ret= setsockopt(con->fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&ret,
                  (socklen_t)sizeof(int));
#else
  ret= setsockopt(con->fd, IPPROTO_TCP, TCP_NODELAY, &ret,
                  (socklen_t)sizeof(int));
#endif /* _WIN32 */

  if (ret == -1 && errno != EOPNOTSUPP)
  {
    drizzle_set_error(con->drizzle, "_con_setsockopt",
                      "setsockopt:TCP_NODELAY:%d", errno);
    return DRIZZLE_RETURN_ERRNO;
  }

  linger.l_onoff= 1;
  linger.l_linger= DRIZZLE_DEFAULT_SOCKET_TIMEOUT;

#ifdef _WIN32
  ret= setsockopt(con->fd, SOL_SOCKET, SO_LINGER, (const char*)&linger,
                  (socklen_t)sizeof(struct linger));
#else
  ret= setsockopt(con->fd, SOL_SOCKET, SO_LINGER, &linger,
                  (socklen_t)sizeof(struct linger));
#endif /* _WIN32 */

  if (ret == -1)
  {
    drizzle_set_error(con->drizzle, "_con_setsockopt",
                      "setsockopt:SO_LINGER:%d", errno);
    return DRIZZLE_RETURN_ERRNO;
  }

  waittime.tv_sec= DRIZZLE_DEFAULT_SOCKET_TIMEOUT;
  waittime.tv_usec= 0;

#ifdef _WIN32
  ret= setsockopt(con->fd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&waittime,
                  (socklen_t)sizeof(struct timeval));
#else
  ret= setsockopt(con->fd, SOL_SOCKET, SO_SNDTIMEO, &waittime,
                  (socklen_t)sizeof(struct timeval));
#endif /* _WIN32 */

  if (ret == -1 && errno != ENOPROTOOPT)
  {
    drizzle_set_error(con->drizzle, "_con_setsockopt",
                      "setsockopt:SO_SNDTIMEO:%d", errno);
    return DRIZZLE_RETURN_ERRNO;
  }

#ifdef _WIN32
  ret= setsockopt(con->fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&waittime,
                  (socklen_t)sizeof(struct timeval));
#else
  ret= setsockopt(con->fd, SOL_SOCKET, SO_RCVTIMEO, &waittime,
                  (socklen_t)sizeof(struct timeval));
#endif /* _WIN32 */

  if (ret == -1 && errno != ENOPROTOOPT)
  {
    drizzle_set_error(con->drizzle, "_con_setsockopt",
                      "setsockopt:SO_RCVTIMEO:%d", errno);
    return DRIZZLE_RETURN_ERRNO;
  }

  ret= DRIZZLE_DEFAULT_SOCKET_SEND_SIZE;
#ifdef _WIN32
  ret= setsockopt(con->fd, SOL_SOCKET, SO_SNDBUF, (const char*)&ret, (socklen_t)sizeof(int));
#else
  ret= setsockopt(con->fd, SOL_SOCKET, SO_SNDBUF, &ret, (socklen_t)sizeof(int));
#endif /* _WIN32 */
  if (ret == -1)
  {
    drizzle_set_error(con->drizzle, "_con_setsockopt",
                      "setsockopt:SO_SNDBUF:%d", errno);
    return DRIZZLE_RETURN_ERRNO;
  }

  ret= DRIZZLE_DEFAULT_SOCKET_RECV_SIZE;
#ifdef _WIN32
  ret= setsockopt(con->fd, SOL_SOCKET, SO_RCVBUF, (const char*)&ret, (socklen_t)sizeof(int));
#else
  ret= setsockopt(con->fd, SOL_SOCKET, SO_RCVBUF, &ret, (socklen_t)sizeof(int));
#endif /* _WIN32 */
  if (ret == -1)
  {
    drizzle_set_error(con->drizzle, "_con_setsockopt",
                      "setsockopt:SO_RCVBUF:%d", errno);
    return DRIZZLE_RETURN_ERRNO;
  }

#if defined (_WIN32)
  unsigned long asyncmode = 1;
  ioctlsocket(con->fd, FIONBIO, &asyncmode);
#else
  ret= fcntl(con->fd, F_GETFL, 0);
  if (ret == -1)
  {
    drizzle_set_error(con->drizzle, "_con_setsockopt", "fcntl:F_GETFL:%d",
                      errno);
    return DRIZZLE_RETURN_ERRNO;
  }

  ret= fcntl(con->fd, F_SETFL, ret | O_NONBLOCK);
  if (ret == -1)
  {
    drizzle_set_error(con->drizzle, "_con_setsockopt", "fcntl:F_SETFL:%d",
                      errno);
    return DRIZZLE_RETURN_ERRNO;
  }
#endif

  return DRIZZLE_RETURN_OK;
}
