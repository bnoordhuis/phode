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
 * @brief Row definitions
 */

#include "common.h"

/*
 * Client definitions
 */

uint64_t drizzle_row_read(drizzle_result_st *result, drizzle_return_t *ret_ptr)
{
  if (drizzle_state_none(result->con))
  {
    drizzle_state_push(result->con, drizzle_state_row_read);
    drizzle_state_push(result->con, drizzle_state_packet_read);
  }

  *ret_ptr= drizzle_state_loop(result->con);

  return result->row_current;
}

drizzle_row_t drizzle_row_buffer(drizzle_result_st *result,
                                 drizzle_return_t *ret_ptr)
{
  size_t total;
  drizzle_field_t field;
  drizzle_row_t row;

  if (result->row == NULL)
  {
    if (drizzle_row_read(result, ret_ptr) == 0 || *ret_ptr != DRIZZLE_RETURN_OK)
      return NULL;

    result->row= malloc((sizeof(drizzle_field_t) + sizeof(size_t)) *
                        result->column_count);
    if (result->row == NULL)
    {
      drizzle_set_error(result->con->drizzle, "drizzle_row_buffer", "malloc");
      *ret_ptr= DRIZZLE_RETURN_MEMORY;
      return NULL;
    }

    result->field_sizes= (size_t *)(result->row + result->column_count);
  }

  while (1)
  {
    field= drizzle_field_buffer(result, &total, ret_ptr);
    if (*ret_ptr == DRIZZLE_RETURN_ROW_END)
      break;
    if (*ret_ptr != DRIZZLE_RETURN_OK)
    {
      if (*ret_ptr != DRIZZLE_RETURN_IO_WAIT)
      {
        free(result->row);
        result->row= NULL;
        result->field_sizes= NULL;
      }

      return NULL;
    }

    result->row[result->field_current - 1]= field;
    result->field_sizes[result->field_current - 1]= total;
  }

  *ret_ptr= DRIZZLE_RETURN_OK;
  row= result->row;
  result->row= NULL;

  return row;
}

void drizzle_row_free(drizzle_result_st *result, drizzle_row_t row)
{
  uint16_t x;

  for (x= 0; x < result->column_count; x++)
      drizzle_field_free(row[x]);

  free(row);
}

size_t *drizzle_row_field_sizes(drizzle_result_st *result)
{
  return result->field_sizes;
}

drizzle_row_t drizzle_row_next(drizzle_result_st *result)
{
  if (result->row_current == result->row_count)
    return NULL;

  result->field_sizes= result->field_sizes_list[result->row_current];
  result->row_current++;
  return result->row_list[result->row_current - 1];
}

drizzle_row_t drizzle_row_prev(drizzle_result_st *result)
{
  if (result->row_current == 0)
    return NULL;

  result->row_current--;
  result->field_sizes= result->field_sizes_list[result->row_current];
  return result->row_list[result->row_current];
}

void drizzle_row_seek(drizzle_result_st *result, uint64_t row)
{
  if (row <= result->row_count)
    result->row_current= row;
}

drizzle_row_t drizzle_row_index(drizzle_result_st *result, uint64_t row)
{
  if (row >= result->row_count)
    return NULL;

  return result->row_list[row];
}

uint64_t drizzle_row_current(drizzle_result_st *result)
{
  return result->row_current;
}

/*
 * Server definitions
 */

drizzle_return_t drizzle_row_write(drizzle_result_st *result)
{
  if (drizzle_state_none(result->con))
    drizzle_state_push(result->con, drizzle_state_row_write);

  return drizzle_state_loop(result->con);
}

/*
 * Internal state functions.
 */

drizzle_return_t drizzle_state_row_read(drizzle_con_st *con)
{
  drizzle_log_debug(con->drizzle, "drizzle_state_row_read");

  if (con->packet_size != 0 && con->buffer_size < con->packet_size)
  {
    drizzle_state_push(con, drizzle_state_read);
    return DRIZZLE_RETURN_OK;
  }

  if (con->packet_size == 5 && con->buffer_ptr[0] == 254)
  {
    /* Got EOF packet, no more rows. */
    con->result->row_current= 0;
    con->result->warning_count= drizzle_get_byte2(con->buffer_ptr + 1);
    con->status= drizzle_get_byte2(con->buffer_ptr + 3);
    con->buffer_ptr+= 5;
    con->buffer_size-= 5;
  }
  else if (con->buffer_ptr[0] == 255)
  {
    drizzle_state_pop(con);
    drizzle_state_push(con, drizzle_state_result_read);
    return DRIZZLE_RETURN_OK;
  }
  else if (con->result->options & DRIZZLE_RESULT_ROW_BREAK)
    con->result->options&= (drizzle_result_options_t)~DRIZZLE_RESULT_ROW_BREAK;
  else
  {
    con->result->row_count++;
    con->result->row_current++;
    con->result->field_current= 0;
  }

  drizzle_state_pop(con);
  return DRIZZLE_RETURN_OK;
}

drizzle_return_t drizzle_state_row_write(drizzle_con_st *con)
{
  uint8_t *start= con->buffer_ptr + con->buffer_size;

  drizzle_log_debug(con->drizzle, "drizzle_state_row_write");

  /* Flush buffer if there is not enough room. */
  if (((size_t)DRIZZLE_MAX_BUFFER_SIZE - (size_t)(start - con->buffer)) < 4)
  {
    drizzle_state_push(con, drizzle_state_write);
    return DRIZZLE_RETURN_OK;
  }

  drizzle_set_byte3(start, con->packet_size);
  start[3]= con->packet_number;
  con->packet_number++;

  con->buffer_size+= 4;

  drizzle_state_pop(con);
  return DRIZZLE_RETURN_OK;
}
