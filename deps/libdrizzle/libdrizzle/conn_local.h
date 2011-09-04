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
 * @brief Local Connection Declarations
 */

#ifndef __DRIZZLE_CONN_LOCAL_H
#define __DRIZZLE_CONN_LOCAL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup drizzle_con_local Local Connection Declarations
 * @ingroup drizzle_con
 * @{
 */

/**
 * Clear address info, freeing structs if needed.
 *
 * @param[in] con Connection structure previously initialized with
 *  drizzle_con_create(), drizzle_con_clone(), or related functions.
 */
DRIZZLE_LOCAL
void drizzle_con_reset_addrinfo(drizzle_con_st *con);
 
/**
 * Check if state stack is empty.
 *
 * @param[in] con Connection structure previously initialized with
 *  drizzle_con_create(), drizzle_con_clone(), or related functions.
 * @return True if empty, false if something is on the stack.
 */
static inline bool drizzle_state_none(drizzle_con_st *con)
{
  return con->state_current == 0;
}

/**
 * Push a function onto the stack.
 *
 * @param[in] con Connection structure previously initialized with
 *  drizzle_con_create(), drizzle_con_clone(), or related functions.
 * @param[in] function Function to push.
 */
static inline void drizzle_state_push(drizzle_con_st *con,
                                      drizzle_state_fn *function)
{
  /* The maximum stack depth can be determined at compile time, so bump this
     constant if needed to avoid the dynamic memory management. */
  assert(con->state_current < DRIZZLE_STATE_STACK_SIZE);
  con->state_stack[con->state_current]= function;
  con->state_current++;
}

/**
 * Pop a function off of the stack.
 *
 * @param[in] con Connection structure previously initialized with
 *  drizzle_con_create(), drizzle_con_clone(), or related functions.
 */
static inline void drizzle_state_pop(drizzle_con_st *con)
{
  con->state_current--;
}

/**
 * Reset the stack so it is empty.
 *
 * @param[in] con Connection structure previously initialized with
 *  drizzle_con_create(), drizzle_con_clone(), or related functions.
 */
static inline void drizzle_state_reset(drizzle_con_st *con)
{
  con->state_current= 0;
}

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __DRIZZLE_CONN_LOCAL_H */
