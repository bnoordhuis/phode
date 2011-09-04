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
 * @brief Tests for drizzle_st Structures
 */

#include <assert.h>

#include "tests/common.h"

#define DRIZZLE_TEST_HOST "127.0.0.1"
#define DRIZZLE_TEST_PORT 12399

typedef enum
{
  SERVER_STATE_START,
  SERVER_STATE_HANDSHAKE_WRITE,
  SERVER_STATE_HANDSHAKE_READ,
  SERVER_STATE_HANDSHAKE_RESULT,
  SERVER_STATE_COMMAND_BUFFER,
  SERVER_STATE_RESULT_HEADER,
  SERVER_STATE_RESULT_COLUMN_1,
  SERVER_STATE_RESULT_COLUMN_2,
  SERVER_STATE_RESULT_COLUMN_EOF,
  SERVER_STATE_RESULT_ROW_1,
  SERVER_STATE_RESULT_ROW_1_FIELD_1,
  SERVER_STATE_RESULT_ROW_1_FIELD_2,
  SERVER_STATE_RESULT_ROW_EOF,
  SERVER_STATE_DONE
} server_state_t;

typedef struct
{
  server_state_t state;
  drizzle_result_st result;
  drizzle_column_st column;
  drizzle_command_t command;
  char *data;
  size_t total;
} server_state_st;

typedef enum
{
  CLIENT_STATE_START,
  CLIENT_STATE_RESULT,
  CLIENT_STATE_DONE
} client_state_t;

typedef struct
{
  client_state_t state;
  drizzle_result_st result;
} client_state_st;

static void _server(drizzle_con_st *con, server_state_st *state);
static void _client(drizzle_con_st *con, client_state_st *state);

int main(void)
{
  drizzle_st drizzle;
  drizzle_con_st listen_con;
  drizzle_con_st client;
  drizzle_con_st server;
  drizzle_return_t ret;
  bool server_accepted = false;
  server_state_st server_state;
  client_state_st client_state;

  drizzle_test("drizzle_create");
  if (drizzle_create(&drizzle) == NULL)
    drizzle_test_error("returned NULL");

  drizzle_test("drizzle_con_add_tcp_listen");
  if (drizzle_con_add_tcp_listen(&drizzle, &listen_con, DRIZZLE_TEST_HOST,
                                 DRIZZLE_TEST_PORT, 1,
                                 DRIZZLE_CON_NONE) == NULL)
  {
    drizzle_test_error("returned NULL");
  }

  drizzle_test("drizzle_con_listen");
  ret= drizzle_con_listen(&listen_con);
  if (ret != DRIZZLE_RETURN_OK)
    drizzle_test_error("returned %s (%d)", drizzle_error(&drizzle), ret);

  drizzle_test("drizzle_con_add_tcp");
  if (drizzle_con_add_tcp(&drizzle, &client, DRIZZLE_TEST_HOST,
                          DRIZZLE_TEST_PORT, NULL, NULL, NULL,
                          DRIZZLE_CON_NONE) == NULL)
  {
    drizzle_test_error("returned NULL");
  }

  drizzle_test("drizzle_add_options");
  drizzle_add_options(&drizzle, DRIZZLE_NON_BLOCKING);

  server_state.state= SERVER_STATE_START;
  client_state.state= CLIENT_STATE_START;

  while (true)
  {
    if (!server_accepted)
    {
      drizzle_test("drizzle_con_accept");
      (void)drizzle_con_accept(&drizzle, &server, &ret);
      if (ret == DRIZZLE_RETURN_OK)
        server_accepted = true;
      else if (ret != DRIZZLE_RETURN_IO_WAIT)
        drizzle_test_error("returned %s (%d)", drizzle_error(&drizzle), ret);
    }

    if (server_accepted)
      _server(&server, &server_state);

    _client(&client, &client_state);

    if (server_state.state == SERVER_STATE_DONE &&
        client_state.state == CLIENT_STATE_DONE)
    {
      break;
    }

    drizzle_test("drizzle_con_wait");
    ret= drizzle_con_wait(&drizzle);
    if (ret != DRIZZLE_RETURN_OK)
      drizzle_test_error("returned %s (%d)", drizzle_error(&drizzle), ret);
  }

  if (server_accepted)
  {
    drizzle_test("drizzle_con_free");
    drizzle_con_free(&server);
  }

  drizzle_test("drizzle_con_free");
  drizzle_con_free(&client);

  drizzle_test("drizzle_con_free");
  drizzle_con_free(&listen_con);

  drizzle_test("drizzle_free");
  drizzle_free(&drizzle);

  return 0;
}

static void _server(drizzle_con_st *con, server_state_st *state)
{
  drizzle_return_t ret;
  const drizzle_field_t const fields[2]=
  {
    (const drizzle_field_t)"test_field_1",
    (const drizzle_field_t)"test_field_2"
  };
  const size_t field_sizes[2]= { 12, 12 };

  switch(state->state)
  {
  case SERVER_STATE_START:
    drizzle_con_set_protocol_version(con, 10);
    drizzle_con_set_server_version(con, "test_version");
    drizzle_con_set_thread_id(con, 1);
    drizzle_con_set_scramble(con, (const uint8_t *)"ABCDEFGHIJKLMNOPQRST");
    drizzle_con_set_capabilities(con, DRIZZLE_CAPABILITIES_NONE);
    drizzle_con_set_charset(con, 8);
    drizzle_con_set_status(con, DRIZZLE_CON_STATUS_NONE);
    drizzle_con_set_max_packet_size(con, DRIZZLE_MAX_PACKET_SIZE);

  case SERVER_STATE_HANDSHAKE_WRITE:
    drizzle_test("drizzle_handshake_server_write");
    ret= drizzle_handshake_server_write(con);
    if (ret == DRIZZLE_RETURN_IO_WAIT)
    {
      state->state = SERVER_STATE_HANDSHAKE_WRITE;
      return;
    }
    else if (ret != DRIZZLE_RETURN_OK)
      drizzle_test_error("returned %s (%d)", drizzle_con_error(con), ret);

  case SERVER_STATE_HANDSHAKE_READ:
    drizzle_test("drizzle_handshake_client_read");
    ret= drizzle_handshake_client_read(con);
    if (ret == DRIZZLE_RETURN_IO_WAIT)
    {
      state->state = SERVER_STATE_HANDSHAKE_READ;
      return;
    }
    else if (ret != DRIZZLE_RETURN_OK)
      drizzle_test_error("returned %s (%d)", drizzle_con_error(con), ret);

    drizzle_test("drizzle_result_create");
    if (drizzle_result_create(con, &state->result) == NULL)
      drizzle_test_error("returned %s", drizzle_con_error(con));

  case SERVER_STATE_HANDSHAKE_RESULT:
    drizzle_test("drizzle_handshake_result_write");
    ret= drizzle_result_write(con, &state->result, true);
    if (ret == DRIZZLE_RETURN_IO_WAIT)
    {
      state->state = SERVER_STATE_HANDSHAKE_RESULT;
      return;
    }
    else if (ret != DRIZZLE_RETURN_OK)
      drizzle_test_error("returned %s (%d)", drizzle_con_error(con), ret);

    drizzle_result_free(&state->result);

  case SERVER_STATE_COMMAND_BUFFER:
    drizzle_test("drizzle_con_command_buffer");
    state->data= drizzle_con_command_buffer(con, &state->command, &state->total,
                                            &ret);
    if (ret == DRIZZLE_RETURN_IO_WAIT)
    {
      state->state = SERVER_STATE_COMMAND_BUFFER;
      return;
    }
    else if (ret != DRIZZLE_RETURN_OK)
      drizzle_test_error("returned %s (%d)", drizzle_con_error(con), ret);

    if (state->command != DRIZZLE_COMMAND_QUERY)
      drizzle_test_error("command was not a query");

    if (state->total != 6 && !memcmp(state->data, "SELECT", 6))
      drizzle_test_error("query doesn't match");

    if (state->data != NULL)
    {
      free(state->data);
      state->data= NULL;
    }

    drizzle_test("drizzle_result_create");
    if (drizzle_result_create(con, &state->result) == NULL)
      drizzle_test_error("returned %s", drizzle_con_error(con));

    drizzle_result_set_column_count(&state->result, 2);

  case SERVER_STATE_RESULT_HEADER:
    drizzle_test("drizzle_handshake_result_write");
    ret= drizzle_result_write(con, &state->result, false);
    if (ret == DRIZZLE_RETURN_IO_WAIT)
    {
      state->state = SERVER_STATE_RESULT_HEADER;
      return;
    }
    else if (ret != DRIZZLE_RETURN_OK)
      drizzle_test_error("returned %s (%d)", drizzle_con_error(con), ret);

    drizzle_test("drizzle_column_create");
    if (drizzle_column_create(&state->result, &state->column) == NULL)
      drizzle_test_error("returned %s", drizzle_con_error(con));

    drizzle_column_set_catalog(&state->column, "test_catalog");
    drizzle_column_set_db(&state->column, "test_db");
    drizzle_column_set_table(&state->column, "test_table");
    drizzle_column_set_orig_table(&state->column, "test_orig_table");
    drizzle_column_set_name(&state->column, "test_column_1");
    drizzle_column_set_orig_name(&state->column, "test_orig_column_1");
    drizzle_column_set_charset(&state->column, 8);
    drizzle_column_set_size(&state->column, 32);
    drizzle_column_set_type(&state->column, DRIZZLE_COLUMN_TYPE_VARCHAR);
    drizzle_column_set_flags(&state->column, DRIZZLE_COLUMN_FLAGS_NONE);

  case SERVER_STATE_RESULT_COLUMN_1:
    drizzle_test("drizzle_column_write");
    ret= drizzle_column_write(&state->result, &state->column);
    if (ret == DRIZZLE_RETURN_IO_WAIT)
    {
      state->state = SERVER_STATE_RESULT_COLUMN_1;
      return;
    }
    else if (ret != DRIZZLE_RETURN_OK)
      drizzle_test_error("returned %s (%d)", drizzle_con_error(con), ret);

    drizzle_column_set_name(&state->column, "test_column_2");
    drizzle_column_set_orig_name(&state->column, "test_orig_column_2");

  case SERVER_STATE_RESULT_COLUMN_2:
    drizzle_test("drizzle_column_write");
    ret= drizzle_column_write(&state->result, &state->column);
    if (ret == DRIZZLE_RETURN_IO_WAIT)
    {
      state->state = SERVER_STATE_RESULT_COLUMN_2;
      return;
    }
    else if (ret != DRIZZLE_RETURN_OK)
      drizzle_test_error("returned %s (%d)", drizzle_con_error(con), ret);

    drizzle_test("drizzle_column_free");
    drizzle_column_free(&state->column);

    drizzle_result_set_eof(&state->result, true);

  case SERVER_STATE_RESULT_COLUMN_EOF:
    drizzle_test("drizzle_handshake_result_write");
    ret= drizzle_result_write(con, &state->result, false);
    if (ret == DRIZZLE_RETURN_IO_WAIT)
    {
      state->state = SERVER_STATE_RESULT_COLUMN_EOF;
      return;
    }
    else if (ret != DRIZZLE_RETURN_OK)
      drizzle_test_error("returned %s (%d)", drizzle_con_error(con), ret);

    drizzle_result_calc_row_size(&state->result, fields, field_sizes);

  case SERVER_STATE_RESULT_ROW_1:
    ret= drizzle_row_write(&state->result);

  case SERVER_STATE_RESULT_ROW_1_FIELD_1:
    ret= drizzle_field_write(&state->result, fields[0], field_sizes[0],
                             field_sizes[0]);

  case SERVER_STATE_RESULT_ROW_1_FIELD_2:
    ret= drizzle_field_write(&state->result, fields[1], field_sizes[1],
                             field_sizes[1]);

  case SERVER_STATE_RESULT_ROW_EOF:
    drizzle_test("drizzle_handshake_result_write");
    ret= drizzle_result_write(con, &state->result, true);
    if (ret == DRIZZLE_RETURN_IO_WAIT)
    {
      state->state = SERVER_STATE_RESULT_ROW_EOF;
      return;
    }
    else if (ret != DRIZZLE_RETURN_OK)
      drizzle_test_error("returned %s (%d)", drizzle_con_error(con), ret);

    drizzle_result_free(&state->result);

    state->state = SERVER_STATE_DONE;

  case SERVER_STATE_DONE:
    return;

  default:
    drizzle_test_error("invalid server state");
  }
}

static void _client(drizzle_con_st *con, client_state_st *state)
{
  drizzle_return_t ret;
  drizzle_column_st *column;
  drizzle_row_t row;
  size_t *field_sizes;

  switch(state->state)
  {
  case CLIENT_STATE_START:
    drizzle_test("drizzle_query_str");
    (void)drizzle_query_str(con, &state->result, "SELECT", &ret);
    if (ret == DRIZZLE_RETURN_IO_WAIT)
    {
      state->state = CLIENT_STATE_START;
      return;
    }
    else if (ret != DRIZZLE_RETURN_OK)
      drizzle_test_error("returned %s (%d)", drizzle_con_error(con), ret);

  case CLIENT_STATE_RESULT:
    drizzle_test("drizzle_result_buffer");
    ret = drizzle_result_buffer(&state->result);
    if (ret == DRIZZLE_RETURN_IO_WAIT)
    {
      state->state = CLIENT_STATE_RESULT;
      return;
    }
    else if (ret != DRIZZLE_RETURN_OK)
      drizzle_test_error("returned %s (%d)", drizzle_con_error(con), ret);

    drizzle_test("drizzle_con_protocol_version");
    if (drizzle_con_protocol_version(con) != 10)
      drizzle_test_error("no match");

    drizzle_test("drizzle_con_server_version");
    if (strcmp(drizzle_con_server_version(con), "test_version"))
      drizzle_test_error("no match");

    drizzle_test("drizzle_con_thread_id");
    if (drizzle_con_thread_id(con) != 1)
      drizzle_test_error("no match");

    drizzle_test("drizzle_con_scramble");
    if (memcmp(drizzle_con_scramble(con), "ABCDEFGHIJKLMNOPQRST",
               DRIZZLE_MAX_SCRAMBLE_SIZE))
    {
      drizzle_test_error("no match");
    }

    /* We default to MySQL protocol right now, which sets this flag. */
    drizzle_test("drizzle_con_capabilities");
    if (drizzle_con_capabilities(con) != DRIZZLE_CAPABILITIES_PROTOCOL_41)
      drizzle_test_error("no match");

    drizzle_test("drizzle_con_charset");
    if (drizzle_con_charset(con) != 8)
      drizzle_test_error("no match");

    drizzle_test("drizzle_con_status");
    if (drizzle_con_status(con) != DRIZZLE_CON_STATUS_NONE)
      drizzle_test_error("no match");

    drizzle_test("drizzle_con_packet_size");
    if (drizzle_con_max_packet_size(con) != DRIZZLE_MAX_PACKET_SIZE)
      drizzle_test_error("no match");

    drizzle_test("drizzle_column_next");
    column= drizzle_column_next(&state->result);
    if (column == NULL)
      drizzle_test_error("column not found");

    drizzle_test("drizzle_column_drizzle_result");
    if (drizzle_column_drizzle_result(column) != &state->result)
      drizzle_test_error("no match");

    drizzle_test("drizzle_column_catalog");
    if (strcmp(drizzle_column_catalog(column), "test_catalog"))
      drizzle_test_error("no match");

    drizzle_test("drizzle_column_db");
    if (strcmp(drizzle_column_db(column), "test_db"))
      drizzle_test_error("no match");

    drizzle_test("drizzle_column_table");
    if (strcmp(drizzle_column_table(column), "test_table"))
      drizzle_test_error("no match");

    drizzle_test("drizzle_column_orig_table");
    if (strcmp(drizzle_column_orig_table(column), "test_orig_table"))
      drizzle_test_error("no match");

    drizzle_test("drizzle_column_name");
    if (strcmp(drizzle_column_name(column), "test_column_1"))
      drizzle_test_error("no match");

    drizzle_test("drizzle_column_orig_name");
    if (strcmp(drizzle_column_orig_name(column), "test_orig_column_1"))
      drizzle_test_error("no match");

    drizzle_test("drizzle_column_charset");
    if (drizzle_column_charset(column) != 8)
      drizzle_test_error("no match");

    drizzle_test("drizzle_column_size");
    if (drizzle_column_size(column) != 32)
      drizzle_test_error("no match");

    drizzle_test("drizzle_column_max_size");
    if (drizzle_column_max_size(column) != 12)
      drizzle_test_error("no match");

    drizzle_test("drizzle_column_type_drizzle");
    if (drizzle_column_type_drizzle(column) != DRIZZLE_COLUMN_TYPE_DRIZZLE_VARCHAR)
      drizzle_test_error("no match");

    drizzle_test("drizzle_column_flags");
    if (drizzle_column_flags(column) != DRIZZLE_COLUMN_FLAGS_NONE)
      drizzle_test_error("no match");

    drizzle_test("drizzle_column_next");
    column= drizzle_column_next(&state->result);
    if (column == NULL)
      drizzle_test_error("column not found");

    drizzle_test("drizzle_column_name");
    if (strcmp(drizzle_column_name(column), "test_column_2"))
      drizzle_test_error("no match");

    drizzle_test("drizzle_column_next");
    column= drizzle_column_next(&state->result);
    if (column != NULL)
      drizzle_test_error("column found");

    drizzle_test("drizzle_column_prev");
    column= drizzle_column_prev(&state->result);
    if (column == NULL)
      drizzle_test_error("column not found");

    drizzle_test("drizzle_column_name");
    if (strcmp(drizzle_column_name(column), "test_column_2"))
      drizzle_test_error("no match");

    drizzle_test("drizzle_row_next");
    row= drizzle_row_next(&state->result);
    if (row == NULL)
      drizzle_test_error("row not found");

    if (strcmp(row[0], "test_field_1") || strcmp(row[1], "test_field_2"))
      drizzle_test_error("no match");

    drizzle_test("drizzle_row_field_sizes");
    field_sizes= drizzle_row_field_sizes(&state->result);
    if (field_sizes[0] != 12 || field_sizes[1] != 12)
      drizzle_test_error("no match");

    drizzle_test("drizzle_row_prev");
    row = drizzle_row_prev(&state->result);
    if (row == NULL)
      drizzle_test_error("row not found");

    if (strcmp(row[0], "test_field_1") || strcmp(row[1], "test_field_2"))
      drizzle_test_error("no match");

    drizzle_test("drizzle_row_next");
    row = drizzle_row_next(&state->result);

    drizzle_test("drizzle_row_index");
    row = drizzle_row_index(&state->result, 0);
    if (row == NULL)
      drizzle_test_error("row not found");

    if (strcmp(row[0], "test_field_1") || strcmp(row[1], "test_field_2"))
      drizzle_test_error("no match");

    drizzle_test("drizzle_row_index");
    row = drizzle_row_index(&state->result, 1);
    if (row != NULL)
      drizzle_test_error("row found");

    drizzle_test("drizzle_result_free");
    drizzle_result_free(&state->result);

    state->state = CLIENT_STATE_DONE;

  case CLIENT_STATE_DONE:
    return;

  default:
    drizzle_test_error("invalid client state");
  }
}
