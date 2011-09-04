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
 * @brief Drizzle Definitions
 */

#include "common.h"


/**
 * @addtogroup drizzle_static Static Drizzle Declarations
 * @ingroup drizzle
 * @{
 */

/**
 * Names of the verbose levels provided.
 */
static const char *_verbose_name[DRIZZLE_VERBOSE_MAX]=
{
  "NEVER",
  "FATAL",
  "ERROR",
  "INFO",
  "DEBUG",
  "CRAZY"
};

/** @} */

/*
 * Common Definitions
 */

const char *drizzle_version(void)
{
  return PACKAGE_VERSION;
}

const char *drizzle_bugreport(void)
{
  return PACKAGE_BUGREPORT;
}

const char *drizzle_verbose_name(drizzle_verbose_t verbose)
{
  if (verbose >= DRIZZLE_VERBOSE_MAX)
    return "UNKNOWN";

  return _verbose_name[verbose];
}

drizzle_st *drizzle_create(drizzle_st *drizzle)
{
#if defined(_WIN32)
  /* if it is MS windows, invoke WSAStartup */
  WSADATA wsaData;
  if ( WSAStartup( MAKEWORD(2,2), &wsaData ) != 0 )
    printf("Error at WSAStartup()\n");
#endif
  if (drizzle == NULL)
  {
    drizzle= malloc(sizeof(drizzle_st));
    if (drizzle == NULL)
      return NULL;

    drizzle->options= DRIZZLE_ALLOCATED;
  }
  else
    drizzle->options= DRIZZLE_NONE;

  /* @todo remove this default free flag with new API. */
  drizzle->options|= DRIZZLE_FREE_OBJECTS;
  drizzle->error_code= 0;
  /* drizzle->options set above */
  drizzle->verbose= DRIZZLE_VERBOSE_NEVER;
  drizzle->con_count= 0;
  drizzle->pfds_size= 0;
  drizzle->query_count= 0;
  drizzle->query_new= 0;
  drizzle->query_running= 0;
  drizzle->last_errno= 0;
  drizzle->timeout= -1;
  drizzle->con_list= NULL;
  drizzle->context= NULL;
  drizzle->context_free_fn= NULL;
  drizzle->event_watch_fn= NULL;
  drizzle->event_watch_context= NULL;
  drizzle->log_fn= NULL;
  drizzle->log_context= NULL;
  drizzle->pfds= NULL;
  drizzle->query_list= NULL;
  drizzle->sqlstate[0]= 0;
  drizzle->last_error[0]= 0;

  return drizzle;
}

drizzle_st *drizzle_clone(drizzle_st *drizzle, const drizzle_st *from)
{
  drizzle_con_st *con;

  drizzle= drizzle_create(drizzle);
  if (drizzle == NULL)
    return NULL;

  drizzle->options|= (from->options & (drizzle_options_t)~DRIZZLE_ALLOCATED);

  for (con= from->con_list; con != NULL; con= con->next)
  {
    if (drizzle_con_clone(drizzle, NULL, con) == NULL)
    {
      drizzle_free(drizzle);
      return NULL;
    }
  }

  return drizzle;
}

void drizzle_free(drizzle_st *drizzle)
{
  if (drizzle->context != NULL && drizzle->context_free_fn != NULL)
    drizzle->context_free_fn(drizzle, drizzle->context);

  if (drizzle->options & DRIZZLE_FREE_OBJECTS)
  {
    drizzle_con_free_all(drizzle);
    drizzle_query_free_all(drizzle);
  }
  else if (drizzle->options & DRIZZLE_ASSERT_DANGLING)
  {
    assert(drizzle->con_list == NULL);
    assert(drizzle->con_list == NULL);
  }

  if (drizzle->pfds != NULL)
    free(drizzle->pfds);

  if (drizzle->options & DRIZZLE_ALLOCATED)
    free(drizzle);
#if defined(_WIN32)
  /* if it is MS windows, invoke WSACleanup() at the end*/
  WSACleanup();
#endif
}

const char *drizzle_error(const drizzle_st *drizzle)
{
  return (const char *)drizzle->last_error;
}

int drizzle_errno(const drizzle_st *drizzle)
{
  return drizzle->last_errno;
}

uint16_t drizzle_error_code(const drizzle_st *drizzle)
{
  return drizzle->error_code;
}

const char *drizzle_sqlstate(const drizzle_st *drizzle)
{
  return drizzle->sqlstate;
}

drizzle_options_t drizzle_options(const drizzle_st *drizzle)
{
  return drizzle->options;
}

void drizzle_set_options(drizzle_st *drizzle, drizzle_options_t options)
{
  drizzle->options= options;
}

void drizzle_add_options(drizzle_st *drizzle, drizzle_options_t options)
{
  drizzle->options|= options;
}

void drizzle_remove_options(drizzle_st *drizzle, drizzle_options_t options)
{
  drizzle->options&= ~options;
}

void *drizzle_context(const drizzle_st *drizzle)
{
  return drizzle->context;
}

void drizzle_set_context(drizzle_st *drizzle, void *context)
{
  drizzle->context= context;
}

void drizzle_set_context_free_fn(drizzle_st *drizzle,
                                 drizzle_context_free_fn *function)
{
  drizzle->context_free_fn= function;
}

int drizzle_timeout(const drizzle_st *drizzle)
{
  return drizzle->timeout;
}

void drizzle_set_timeout(drizzle_st *drizzle, int timeout)
{
  drizzle->timeout= timeout;
}

drizzle_verbose_t drizzle_verbose(const drizzle_st *drizzle)
{
  return drizzle->verbose;
}

void drizzle_set_verbose(drizzle_st *drizzle, drizzle_verbose_t verbose)
{
  drizzle->verbose= verbose;
}

void drizzle_set_log_fn(drizzle_st *drizzle, drizzle_log_fn *function,
                        void *context)
{
  drizzle->log_fn= function;
  drizzle->log_context= context;
}

void drizzle_set_event_watch_fn(drizzle_st *drizzle,
                                drizzle_event_watch_fn *function,
                                void *context)
{
  drizzle->event_watch_fn= function;
  drizzle->event_watch_context= context;
}

drizzle_con_st *drizzle_con_create(drizzle_st *drizzle, drizzle_con_st *con)
{
  if (con == NULL)
  {
    con= malloc(sizeof(drizzle_con_st));
    if (con == NULL)
    {
      if (drizzle != NULL)
        drizzle_set_error(drizzle, "drizzle_con_create", "malloc");
      return NULL;
    }

    con->options= DRIZZLE_CON_ALLOCATED;
  }
  else
    con->options= 0;

  if (drizzle->con_list != NULL)
    drizzle->con_list->prev= con;
  con->next= drizzle->con_list;
  con->prev= NULL;
  drizzle->con_list= con;
  drizzle->con_count++;

  con->packet_number= 0;
  con->protocol_version= 0;
  con->state_current= 0;
  con->events= 0;
  con->revents= 0;
  con->capabilities= DRIZZLE_CAPABILITIES_NONE;
  con->charset= 0;
  con->command= 0;
  con->options|= DRIZZLE_CON_MYSQL;
  con->socket_type= DRIZZLE_CON_SOCKET_TCP;
  con->status= DRIZZLE_CON_STATUS_NONE;
  con->max_packet_size= DRIZZLE_MAX_PACKET_SIZE;
  con->result_count= 0;
  con->thread_id= 0;
  con->backlog= DRIZZLE_DEFAULT_BACKLOG;
  con->fd= -1;
  con->buffer_size= 0;
  con->command_offset= 0;
  con->command_size= 0;
  con->command_total= 0;
  con->packet_size= 0;
  con->addrinfo_next= NULL;
  con->buffer_ptr= con->buffer;
  con->command_buffer= NULL;
  con->command_data= NULL;
  con->context= NULL;
  con->context_free_fn= NULL;
  con->drizzle= drizzle;
  /* con->next set above */
  /* con->prev set above */
  con->query= NULL;
  /* con->result doesn't need to be set */
  con->result_list= NULL;
  con->scramble= NULL;
  con->socket.tcp.addrinfo= NULL;
  con->socket.tcp.host= NULL;
  con->socket.tcp.port= 0;
  /* con->buffer doesn't need to be set */
  con->db[0]= 0;
  con->password[0]= 0;
  /* con->scramble_buffer doesn't need to be set */
  con->server_version[0]= 0;
  /* con->state_stack doesn't need to be set */
  con->user[0]= 0;

  return con;
}

drizzle_con_st *drizzle_con_clone(drizzle_st *drizzle, drizzle_con_st *con,
                                  const drizzle_con_st *from)
{
  con= drizzle_con_create(drizzle, con);
  if (con == NULL)
    return NULL;

  /* Clear "operational" options such as IO status. */
  con->options|= (from->options & (drizzle_con_options_t)~(
                  DRIZZLE_CON_ALLOCATED|DRIZZLE_CON_READY|
                  DRIZZLE_CON_NO_RESULT_READ|DRIZZLE_CON_IO_READY|
                  DRIZZLE_CON_LISTEN));
  con->backlog= from->backlog;
  strcpy(con->db, from->db);
  strcpy(con->password, from->password);
  strcpy(con->user, from->user);

  switch (from->socket_type)
  {
  case DRIZZLE_CON_SOCKET_TCP:
    drizzle_con_set_tcp(con, from->socket.tcp.host, from->socket.tcp.port);
    break;

  case DRIZZLE_CON_SOCKET_UDS:
    drizzle_con_set_uds(con, from->socket.uds.sockaddr.sun_path);
    break;

  default:
    break;
  }

  return con;
}

void drizzle_con_free(drizzle_con_st *con)
{
  if (con->context != NULL && con->context_free_fn != NULL)
    con->context_free_fn(con, con->context);

  if (con->drizzle->options & DRIZZLE_FREE_OBJECTS)
    drizzle_result_free_all(con);
  else if (con->drizzle->options & DRIZZLE_ASSERT_DANGLING)
    assert(con->result_list == NULL);

  if (con->fd != -1)
    drizzle_con_close(con);

  drizzle_con_reset_addrinfo(con);

  if (con->drizzle->con_list == con)
    con->drizzle->con_list= con->next;
  if (con->prev != NULL)
    con->prev->next= con->next;
  if (con->next != NULL)
    con->next->prev= con->prev;
  con->drizzle->con_count--;

  if (con->options & DRIZZLE_CON_ALLOCATED)
    free(con);
}

void drizzle_con_free_all(drizzle_st *drizzle)
{
  while (drizzle->con_list != NULL)
    drizzle_con_free(drizzle->con_list);
}

drizzle_return_t drizzle_con_wait(drizzle_st *drizzle)
{
  drizzle_con_st *con;
  struct pollfd *pfds;
  uint32_t x;
  int ret;
  drizzle_return_t dret;

  if (drizzle->pfds_size < drizzle->con_count)
  {
    pfds= realloc(drizzle->pfds, drizzle->con_count * sizeof(struct pollfd));
    if (pfds == NULL)
    {
      drizzle_set_error(drizzle, "drizzle_con_wait", "realloc");
      return DRIZZLE_RETURN_MEMORY;
    }

    drizzle->pfds= pfds;
    drizzle->pfds_size= drizzle->con_count;
  }
  else
    pfds= drizzle->pfds;

  x= 0;
  for (con= drizzle->con_list; con != NULL; con= con->next)
  {
    if (con->events == 0)
      continue;

    pfds[x].fd= con->fd;
    pfds[x].events= con->events;
    pfds[x].revents= 0;
    x++;
  }

  if (x == 0)
  {
    drizzle_set_error(drizzle, "drizzle_con_wait",
                      "no active file descriptors");
    return DRIZZLE_RETURN_NO_ACTIVE_CONNECTIONS;
  }

  while (1)
  {
    drizzle_log_crazy(drizzle, "poll count=%d timeout=%d", x,
                      drizzle->timeout);

    ret= poll(pfds, x, drizzle->timeout);

    drizzle_log_crazy(drizzle, "poll return=%d errno=%d", ret, errno);

    if (ret == -1)
    {
      if (errno == EINTR)
        continue;

      drizzle_set_error(drizzle, "drizzle_con_wait", "poll:%d", errno);
      drizzle->last_errno= errno;
      return DRIZZLE_RETURN_ERRNO;
    }

    break;
  }

  if (ret == 0)
  {
    drizzle_set_error(drizzle, "drizzle_con_wait", "timeout reached");
    return DRIZZLE_RETURN_TIMEOUT;
  }

  x= 0;
  for (con= drizzle->con_list; con != NULL; con= con->next)
  {
    if (con->events == 0)
      continue;

    dret= drizzle_con_set_revents(con, pfds[x].revents);
    if (dret != DRIZZLE_RETURN_OK)
      return dret;

    x++;
  }

  return DRIZZLE_RETURN_OK;
}

drizzle_con_st *drizzle_con_ready(drizzle_st *drizzle)
{
  drizzle_con_st *con;

  /* We can't keep state between calls since connections may be removed during
     processing. If this list ever gets big, we may want something faster. */

  for (con= drizzle->con_list; con != NULL; con= con->next)
  {
    if (con->options & DRIZZLE_CON_IO_READY)
    {
      con->options&= (drizzle_con_options_t)~DRIZZLE_CON_IO_READY;
      return con;
    }
  }

  return NULL;
}

drizzle_con_st *drizzle_con_ready_listen(drizzle_st *drizzle)
{
  drizzle_con_st *con;

  /* We can't keep state between calls since connections may be removed during
     processing. If this list ever gets big, we may want something faster. */

  for (con= drizzle->con_list; con != NULL; con= con->next)
  {
    if ((con->options & (DRIZZLE_CON_IO_READY | DRIZZLE_CON_LISTEN)) ==
        (DRIZZLE_CON_IO_READY | DRIZZLE_CON_LISTEN))
    {
      con->options&= (drizzle_con_options_t)~DRIZZLE_CON_IO_READY;
      return con;
    }
  }

  return NULL;
}

/*
 * Client Definitions
 */

drizzle_con_st *drizzle_con_add_tcp(drizzle_st *drizzle, drizzle_con_st *con,
                                    const char *host, in_port_t port,
                                    const char *user, const char *password,
                                    const char *db,
                                    drizzle_con_options_t options)
{
  con= drizzle_con_create(drizzle, con);
  if (con == NULL)
    return NULL;

  drizzle_con_set_tcp(con, host, port);
  drizzle_con_set_auth(con, user, password);
  drizzle_con_set_db(con, db);
  drizzle_con_add_options(con, options);

  return con;
}

drizzle_con_st *drizzle_con_add_uds(drizzle_st *drizzle, drizzle_con_st *con,
                                    const char *uds, const char *user,
                                    const char *password, const char *db,
                                    drizzle_con_options_t options)
{
  con= drizzle_con_create(drizzle, con);
  if (con == NULL)
    return NULL;

  drizzle_con_set_uds(con, uds);
  drizzle_con_set_auth(con, user, password);
  drizzle_con_set_db(con, db);
  drizzle_con_add_options(con, options);

  return con;
}

/*
 * Server Definitions
 */

drizzle_con_st *drizzle_con_add_tcp_listen(drizzle_st *drizzle,
                                           drizzle_con_st *con,
                                           const char *host, in_port_t port,
                                           int backlog,
                                           drizzle_con_options_t options)
{
  con= drizzle_con_create(drizzle, con);
  if (con == NULL)
    return NULL;

  drizzle_con_set_tcp(con, host, port);
  drizzle_con_set_backlog(con, backlog);
  drizzle_con_add_options(con, DRIZZLE_CON_LISTEN | options);

  return con;
}

drizzle_con_st *drizzle_con_add_uds_listen(drizzle_st *drizzle,
                                           drizzle_con_st *con,
                                           const char *uds, int backlog,
                                           drizzle_con_options_t options)
{
  con= drizzle_con_create(drizzle, con);
  if (con == NULL)
    return NULL;

  drizzle_con_set_uds(con, uds);
  drizzle_con_set_backlog(con, backlog);
  drizzle_con_add_options(con, DRIZZLE_CON_LISTEN | options);

  return con;
}

drizzle_con_st *drizzle_con_accept(drizzle_st *drizzle, drizzle_con_st *con,
                                   drizzle_return_t *ret_ptr)
{
  drizzle_con_st *ready;
  int fd;

  while (1)
  {
    if ((ready= drizzle_con_ready_listen(drizzle)) != NULL)
    {
      fd= accept(ready->fd, NULL, NULL);

      con= drizzle_con_create(drizzle, con);
      if (con == NULL)
      {
        (void)close(fd);
        *ret_ptr= DRIZZLE_RETURN_MEMORY;
        return NULL;
      }

      *ret_ptr= drizzle_con_set_fd(con, fd);
      if (*ret_ptr != DRIZZLE_RETURN_OK)
      {
        (void)close(fd);
        return NULL;
      }

      if (ready->options & DRIZZLE_CON_MYSQL)
        drizzle_con_add_options(con, DRIZZLE_CON_MYSQL);

      *ret_ptr= DRIZZLE_RETURN_OK;
      return con;
    }

    if (drizzle->options & DRIZZLE_NON_BLOCKING)
    {
      *ret_ptr= DRIZZLE_RETURN_IO_WAIT;
      return NULL;
    }

    for (ready= drizzle->con_list; ready != NULL; ready= ready->next)
    {
      if (ready->options & DRIZZLE_CON_LISTEN)
        drizzle_con_set_events(ready, POLLIN);
    }

    *ret_ptr= drizzle_con_wait(drizzle);
    if (*ret_ptr != DRIZZLE_RETURN_OK)
      return NULL;
  }
}

/*
 * Local Definitions
 */

void drizzle_set_error(drizzle_st *drizzle, const char *function,
                       const char *format, ...)
{
  size_t size;
  char *ptr;
  char log_buffer[DRIZZLE_MAX_ERROR_SIZE];
  va_list args;

  size= strlen(function);
  ptr= memcpy(log_buffer, function, size);
  ptr+= size;
  ptr[0]= ':';
  size++;
  ptr++;

  va_start(args, format);
  size+= (size_t)vsnprintf(ptr, DRIZZLE_MAX_ERROR_SIZE - size, format, args);
  va_end(args);

  if (drizzle->log_fn == NULL)
  {
    if (size >= DRIZZLE_MAX_ERROR_SIZE)
      size= DRIZZLE_MAX_ERROR_SIZE - 1;

    memcpy(drizzle->last_error, log_buffer, size + 1);
  }
  else
    drizzle->log_fn(log_buffer, DRIZZLE_VERBOSE_ERROR, drizzle->log_context);
}

void drizzle_log(drizzle_st *drizzle, drizzle_verbose_t verbose,
                 const char *format, va_list args)
{
  char log_buffer[DRIZZLE_MAX_ERROR_SIZE];

  if (drizzle->log_fn == NULL)
  {
    printf("%5s: ", drizzle_verbose_name(verbose));
    vprintf(format, args);
    printf("\n");
  }
  else
  {
    vsnprintf(log_buffer, DRIZZLE_MAX_ERROR_SIZE, format, args);
    drizzle->log_fn(log_buffer, verbose, drizzle->log_context);
  }
}
