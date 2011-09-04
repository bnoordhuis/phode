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
 * @brief State Machine Declarations
 */

#ifndef __DRIZZLE_STATE_H
#define __DRIZZLE_STATE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup drizzle_state State Machine Declarations
 *
 * These functions are used in the protocol parsing state machine. Not all
 * functions are defined in state.c, they are in the most appropriate source
 * file (for example, handshake.c for drizzle_state_handshake_server_read).
 * @{
 */

/**
 * Main state loop for connections.
 *
 * @param[in] con Connection structure previously initialized with
 *  drizzle_con_create(), drizzle_con_clone(), or related functions.
 * @return Standard drizzle return value.
 */
drizzle_return_t drizzle_state_loop(drizzle_con_st *con);

/* Functions in state.c */
drizzle_return_t drizzle_state_packet_read(drizzle_con_st *con);

/* Functions in conn.c */
drizzle_return_t drizzle_state_addrinfo(drizzle_con_st *con);
drizzle_return_t drizzle_state_connect(drizzle_con_st *con);
drizzle_return_t drizzle_state_connecting(drizzle_con_st *con);
drizzle_return_t drizzle_state_read(drizzle_con_st *con);
drizzle_return_t drizzle_state_write(drizzle_con_st *con);
drizzle_return_t drizzle_state_listen(drizzle_con_st *con);

/* Functions in handshake.c */
drizzle_return_t drizzle_state_handshake_server_read(drizzle_con_st *con);
drizzle_return_t drizzle_state_handshake_server_write(drizzle_con_st *con);
drizzle_return_t drizzle_state_handshake_client_read(drizzle_con_st *con);
drizzle_return_t drizzle_state_handshake_client_write(drizzle_con_st *con);
drizzle_return_t drizzle_state_handshake_result_read(drizzle_con_st *con);

/* Functions in command.c */
drizzle_return_t drizzle_state_command_read(drizzle_con_st *con);
drizzle_return_t drizzle_state_command_write(drizzle_con_st *con);

/* Functions in result.c */
drizzle_return_t drizzle_state_result_read(drizzle_con_st *con);
drizzle_return_t drizzle_state_result_write(drizzle_con_st *con);

/* Functions in column.c */
drizzle_return_t drizzle_state_column_read(drizzle_con_st *con);
drizzle_return_t drizzle_state_column_write(drizzle_con_st *con);

/* Functions in row.c */
drizzle_return_t drizzle_state_row_read(drizzle_con_st *con);
drizzle_return_t drizzle_state_row_write(drizzle_con_st *con);

/* Functions in field.c */
drizzle_return_t drizzle_state_field_read(drizzle_con_st *con);
drizzle_return_t drizzle_state_field_write(drizzle_con_st *con);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __DRIZZLE_STATE_H */
