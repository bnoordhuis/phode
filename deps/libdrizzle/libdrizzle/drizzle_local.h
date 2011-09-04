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
 * @brief Local Drizzle Declarations
 */

#ifndef __DRIZZLE_LOCAL_H
#define __DRIZZLE_LOCAL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup drizzle_local Local Drizzle Declarations
 * @ingroup drizzle
 * @{
 */

/**
 * Set the error string.
 *
 * @param[in] drizzle Drizzle structure previously initialized with
 *  drizzle_create() or drizzle_clone().
 * @param[in] function Name of function the error happened in. 
 * @param[in] format Format and variable argument list of message.
 */
DRIZZLE_LOCAL
void drizzle_set_error(drizzle_st *drizzle, const char *function,
                       const char *format, ...);

/**
 * Log a message.
 *
 * @param[in] drizzle Drizzle structure previously initialized with
 *  drizzle_create() or drizzle_clone().
 * @param[in] verbose Logging level of the message.
 * @param[in] format Format and variable argument list of message.
 * @param[in] args Variable argument list that has been initialized.
 */
DRIZZLE_LOCAL
void drizzle_log(drizzle_st *drizzle, drizzle_verbose_t verbose,
                 const char *format, va_list args);

/**
 * Log a fatal message, see drizzle_log() for argument details.
 */
static inline void drizzle_log_fatal(drizzle_st *drizzle, const char *format,
                                     ...)
{
  va_list args;

  if (drizzle->verbose >= DRIZZLE_VERBOSE_FATAL)
  {
    va_start(args, format);
    drizzle_log(drizzle, DRIZZLE_VERBOSE_FATAL, format, args);
    va_end(args);
  }
}

/**
 * Log an error message, see drizzle_log() for argument details.
 */
static inline void drizzle_log_error(drizzle_st *drizzle, const char *format,
                                     ...)
{
  va_list args;

  if (drizzle->verbose >= DRIZZLE_VERBOSE_ERROR)
  {
    va_start(args, format);
    drizzle_log(drizzle, DRIZZLE_VERBOSE_ERROR, format, args);
    va_end(args);
  }
}

/**
 * Log an info message, see drizzle_log() for argument details.
 */
static inline void drizzle_log_info(drizzle_st *drizzle, const char *format,
                                    ...)
{
  va_list args;

  if (drizzle->verbose >= DRIZZLE_VERBOSE_INFO)
  {
    va_start(args, format);
    drizzle_log(drizzle, DRIZZLE_VERBOSE_INFO, format, args);
    va_end(args);
  }
}

/**
 * Log a debug message, see drizzle_log() for argument details.
 */
static inline void drizzle_log_debug(drizzle_st *drizzle, const char *format,
                                     ...)
{
  va_list args;

  if (drizzle->verbose >= DRIZZLE_VERBOSE_DEBUG)
  {
    va_start(args, format);
    drizzle_log(drizzle, DRIZZLE_VERBOSE_DEBUG, format, args);
    va_end(args);
  }
}

/**
 * Log a crazy message, see drizzle_log() for argument details.
 */
static inline void drizzle_log_crazy(drizzle_st *drizzle, const char *format,
                                     ...)
{
  va_list args;

  if (drizzle->verbose >= DRIZZLE_VERBOSE_CRAZY)
  {
    va_start(args, format);
    drizzle_log(drizzle, DRIZZLE_VERBOSE_CRAZY, format, args);
    va_end(args);
  }
}

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __DRIZZLE_LOCAL_H */
