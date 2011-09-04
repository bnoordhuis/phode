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
 * @brief Query definitions
 */

#include "common.h"

drizzle_result_st *drizzle_query(drizzle_con_st *con, drizzle_result_st *result,
                                 const char *query, size_t size,
                                 drizzle_return_t *ret_ptr)
{
  return drizzle_con_command_write(con, result, DRIZZLE_COMMAND_QUERY,
                                   (uint8_t *)query, size, size, ret_ptr);
}

drizzle_result_st *drizzle_query_str(drizzle_con_st *con,
                                     drizzle_result_st *result,
                                     const char *query, 
                                     drizzle_return_t *ret_ptr)
{
  size_t size;

  size= strlen(query);

  return drizzle_con_command_write(con, result, DRIZZLE_COMMAND_QUERY,
                                   (uint8_t *)query, size, size, ret_ptr);
}

drizzle_result_st *drizzle_query_inc(drizzle_con_st *con,
                                     drizzle_result_st *result,
                                     const char *query, size_t size,
                                     size_t total, drizzle_return_t *ret_ptr)
{
  return drizzle_con_command_write(con, result, DRIZZLE_COMMAND_QUERY,
                                   (uint8_t *)query, size, total, ret_ptr);
}

drizzle_query_st *drizzle_query_add(drizzle_st *drizzle,
                                    drizzle_query_st *query,
                                    drizzle_con_st *con,
                                    drizzle_result_st *result,
                                    const char *query_string, size_t size,
                                    drizzle_query_options_t options,
                                    void *context)
{
  query= drizzle_query_create(drizzle, query);
  if (query == NULL)
    return NULL;

  drizzle_query_set_con(query, con);
  drizzle_query_set_result(query, result);
  drizzle_query_set_string(query, query_string, size);
  drizzle_query_add_options(query, options);
  drizzle_query_set_context(query, context);

  return query;
}

drizzle_query_st *drizzle_query_create(drizzle_st *drizzle,
                                       drizzle_query_st *query)
{
  if (query == NULL)
  {
    query= malloc(sizeof(drizzle_query_st));
    if (query == NULL)
    {
      drizzle_set_error(drizzle, "drizzle_query_create", "malloc");
      return NULL;
    }

    memset(query, 0, sizeof(drizzle_query_st));
    query->options|= DRIZZLE_CON_ALLOCATED;
  }
  else
    memset(query, 0, sizeof(drizzle_query_st));

  query->drizzle= drizzle;

  if (drizzle->query_list)
    drizzle->query_list->prev= query;
  query->next= drizzle->query_list;
  drizzle->query_list= query;
  drizzle->query_count++;
  drizzle->query_new++;

  return query;
}

void drizzle_query_free(drizzle_query_st *query)
{
  if (query->context != NULL && query->context_free_fn != NULL)
    query->context_free_fn(query, query->context);

  if (query->drizzle->query_list == query)
    query->drizzle->query_list= query->next;
  if (query->prev)
    query->prev->next= query->next;
  if (query->next)
    query->next->prev= query->prev;
  query->drizzle->query_count--;

  if (query->options & DRIZZLE_QUERY_ALLOCATED)
    free(query);
}

void drizzle_query_free_all(drizzle_st *drizzle)
{
  while (drizzle->query_list != NULL)
    drizzle_query_free(drizzle->query_list);
}

drizzle_con_st *drizzle_query_con(drizzle_query_st *query)
{
  return query->con;
}

void drizzle_query_set_con(drizzle_query_st *query, drizzle_con_st *con)
{
  query->con= con;
}

drizzle_result_st *drizzle_query_result(drizzle_query_st *query)
{
  return query->result;
}

void drizzle_query_set_result(drizzle_query_st *query,
                              drizzle_result_st *result)
{
  query->result= result;
}

char *drizzle_query_string(drizzle_query_st *query, size_t *size)
{
  *size= query->size;
  return (char *)(query->string);
}

void drizzle_query_set_string(drizzle_query_st *query, const char *string,
                              size_t size)
{
  query->string= string;
  query->size= size;
}

drizzle_query_options_t drizzle_query_options(drizzle_query_st *query)
{
  return query->options;
}

void drizzle_query_set_options(drizzle_query_st *query,
                               drizzle_query_options_t options)
{
  query->options= options;
}

void drizzle_query_add_options(drizzle_query_st *query,
                               drizzle_query_options_t options)
{
  query->options|= options;
}

void drizzle_query_remove_options(drizzle_query_st *query,
                                  drizzle_query_options_t options)
{
  query->options&= ~options;
}

void *drizzle_query_context(drizzle_query_st *query)
{
  return query->context;
}

void drizzle_query_set_context(drizzle_query_st *query, void *context)
{
  query->context= context;
}

void drizzle_query_set_context_free_fn(drizzle_query_st *query,
                                       drizzle_query_context_free_fn *function)
{
  query->context_free_fn= function;
}

static void drizzle_query_run_state(drizzle_query_st* query,
                                    drizzle_return_t* ret_ptr)
{
  switch (query->state)
  {
  case DRIZZLE_QUERY_STATE_INIT:
    query->state= DRIZZLE_QUERY_STATE_QUERY;
  case DRIZZLE_QUERY_STATE_QUERY:
    query->result= drizzle_query(query->con, query->result, query->string,
                                 query->size, ret_ptr);
    if (*ret_ptr == DRIZZLE_RETURN_IO_WAIT)
    {
      return;
    }
    else if (*ret_ptr != DRIZZLE_RETURN_OK)
    {
      query->state= DRIZZLE_QUERY_STATE_DONE;
      return;
    }

    query->state= DRIZZLE_QUERY_STATE_RESULT;

  case DRIZZLE_QUERY_STATE_RESULT:
    *ret_ptr= drizzle_result_buffer(query->result);
    if (*ret_ptr == DRIZZLE_RETURN_IO_WAIT)
    {
      return;
    }

    query->state= DRIZZLE_QUERY_STATE_DONE;
    return;

  default:
  case DRIZZLE_QUERY_STATE_DONE:
    return;
  }
}

drizzle_query_st *drizzle_query_run(drizzle_st *drizzle,
                                    drizzle_return_t *ret_ptr)
{
  drizzle_options_t options;
  drizzle_query_st *query;
  drizzle_con_st *con;

  if (drizzle->query_new == 0 && drizzle->query_running == 0)
  {
    *ret_ptr= DRIZZLE_RETURN_OK;
    return NULL;
  }

  options= drizzle->options;
  drizzle->options|= DRIZZLE_NON_BLOCKING;

  /* Check to see if any queries need to be started. */
  if (drizzle->query_new > 0)
  {
    for (query= drizzle->query_list; query != NULL; query= query->next)
    {
      if (query->state != DRIZZLE_QUERY_STATE_INIT)
        continue;

      drizzle->query_new--;
      drizzle->query_running++;
      assert(query->con->query == NULL);
      query->con->query= query;

      drizzle_query_run_state(query, ret_ptr);
      if (*ret_ptr != DRIZZLE_RETURN_IO_WAIT)
      {
        assert(query->state == DRIZZLE_QUERY_STATE_DONE);
        drizzle->query_running--;
        drizzle->options= options;
        query->con->query= NULL;
        if (*ret_ptr == DRIZZLE_RETURN_ERROR_CODE || *ret_ptr == DRIZZLE_RETURN_OK)
        {
          return query;
        }
        return NULL;
      }
    }
    assert(drizzle->query_new == 0);
  }

  while (1)
  {
    /* Loop through each active connection. */
    while ((con= drizzle_con_ready(drizzle)) != NULL)
    {
      query= con->query;
      drizzle_query_run_state(query, ret_ptr);
      if (query->state == DRIZZLE_QUERY_STATE_DONE)
      {
        drizzle->query_running--;
        drizzle->options= options;
        con->query= NULL;
        return query;
      }
      assert(*ret_ptr == DRIZZLE_RETURN_IO_WAIT);
    }

    if (options & DRIZZLE_NON_BLOCKING)
    {
      *ret_ptr= DRIZZLE_RETURN_IO_WAIT;
      return NULL;
    }

    *ret_ptr= drizzle_con_wait(drizzle);
    if (*ret_ptr != DRIZZLE_RETURN_OK)
    {
      drizzle->options= options;
      return NULL;
    }
  }
}

drizzle_return_t drizzle_query_run_all(drizzle_st *drizzle)
{
  drizzle_return_t ret;

  while (drizzle->query_new > 0 || drizzle->query_running > 0)
  {
    (void)drizzle_query_run(drizzle, &ret);
    if (ret != DRIZZLE_RETURN_OK && ret != DRIZZLE_RETURN_ERROR_CODE)
      return ret;
  }

  return DRIZZLE_RETURN_OK;
}

size_t drizzle_escape_string(char *to, const char *from, size_t from_size)
{
  size_t to_size= 0;

  while (from_size > 0)
  {
    /* All multi-byte UTF8 characters have the high bit set for all bytes. */
    if (!(*from & 0x80))
    {
      switch (*from)
      {
      case 0:
      case '\n':
      case '\r':
      case '\\':
      case '\'':
      case '"':
      case '\032':
        *to++= '\\';
        to_size++;
      default:
        break;
      }
    }

    *to++= *from++;
    from_size--;
    to_size++;
  }

  *to= 0;

  return to_size;
}

size_t drizzle_hex_string(char *to, const char *from, size_t from_size)
{
  static const char hex_map[]= "0123456789ABCDEF";
  const char *from_end;

  for (from_end= from + from_size; from != from_end; from++)
  {
    *to++= hex_map[((unsigned char) *from) >> 4];
    *to++= hex_map[((unsigned char) *from) & 0xF];
  }

  *to= 0;

  return from_size * 2;
}

void drizzle_mysql_password_hash(char *to, const char *from, size_t from_size)
{
  SHA1_CTX ctx;
  uint8_t hash_tmp1[SHA1_DIGEST_LENGTH];
  uint8_t hash_tmp2[SHA1_DIGEST_LENGTH];

  SHA1Init(&ctx);
  SHA1Update(&ctx, (const uint8_t*)from, from_size);
  SHA1Final(hash_tmp1, &ctx);

  SHA1Init(&ctx);
  SHA1Update(&ctx, hash_tmp1, SHA1_DIGEST_LENGTH);
  SHA1Final(hash_tmp2, &ctx);

  (void)drizzle_hex_string(to, (char*)hash_tmp2, SHA1_DIGEST_LENGTH);
}
