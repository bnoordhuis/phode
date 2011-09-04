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
 * @brief System Include Files
 */

#ifndef __DRIZZLE_COMMON_H
#define __DRIZZLE_COMMON_H

#include "config.h"

#include "drizzle_client.h"
#include "drizzle_server.h"

#ifdef HAVE_ASSERT_H
#include <assert.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_SYS_UIO_H
#include <sys/uio.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "drizzle_local.h"
#include "conn_local.h"
#include "pack.h"
#include "state.h"
#include "sha1.h"

#endif /* __DRIZZLE_COMMON_H */
