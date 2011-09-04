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
 * @brief State machine definitions
 */

#include "common.h"

drizzle_return_t drizzle_state_loop(drizzle_con_st *con)
{
  drizzle_return_t ret;

  while (!drizzle_state_none(con))
  {
    ret= con->state_stack[con->state_current - 1](con);
    if (ret != DRIZZLE_RETURN_OK)
    {
      if (ret != DRIZZLE_RETURN_IO_WAIT && ret != DRIZZLE_RETURN_PAUSE &&
          ret != DRIZZLE_RETURN_ERROR_CODE)
      {
        drizzle_con_close(con);
      }

      return ret;
    }
  }

  return DRIZZLE_RETURN_OK;
}

drizzle_return_t drizzle_state_packet_read(drizzle_con_st *con)
{
  drizzle_log_debug(con->drizzle, "drizzle_state_packet_read");

  if (con->buffer_size < 4)
  {
    drizzle_state_push(con, drizzle_state_read);
    return DRIZZLE_RETURN_OK;
  }

  con->packet_size= drizzle_get_byte3(con->buffer_ptr);

  if (con->packet_number != con->buffer_ptr[3])
  {
    drizzle_set_error(con->drizzle, "drizzle_state_packet_read",
                      "bad packet number:%u:%u", con->packet_number,
                      con->buffer_ptr[3]);
    return DRIZZLE_RETURN_BAD_PACKET_NUMBER;
  }

  drizzle_log_debug(con->drizzle, "packet_size= %zu, packet_number= %u",
                    con->packet_size, con->packet_number);

  con->packet_number++;

  con->buffer_ptr+= 4;
  con->buffer_size-= 4;

  drizzle_state_pop(con);
  return DRIZZLE_RETURN_OK;
}
