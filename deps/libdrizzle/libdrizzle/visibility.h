/*
 * Drizzle Client & Protocol Library
 *
 * Copyright (C) 2008 Eric Day (eday@oddments.org)
 * All rights reserved.
 *
 * Use and distribution licensed under the BSD license.  See
 * the COPYING file in this directory for full text.
 * 
 * Implementation drawn from visibility.texi in gnulib.
 */

/**
 * @file
 * @brief Visibility Control Macros
 */

#ifndef __DRIZZLE_VISIBILITY_H
#define __DRIZZLE_VISIBILITY_H

/**
 *
 * DRIZZLE_API is used for the public API symbols. It either DLL imports or
 * DLL exports (or does nothing for static build).
 *
 * DRIZZLE_LOCAL is used for non-api symbols.
 */

#if defined(_WIN32)
# define DRIZZLE_API
# define DRIZZLE_LOCAL
#else
#if defined(BUILDING_LIBDRIZZLE)
# if defined(HAVE_VISIBILITY)
#  define DRIZZLE_API __attribute__ ((visibility("default")))
#  define DRIZZLE_LOCAL  __attribute__ ((visibility("hidden")))
# elif defined (__SUNPRO_C) && (__SUNPRO_C >= 0x550)
#  define DRIZZLE_API __global
#  define DRIZZLE_API __hidden
# elif defined(_MSC_VER)
#  define DRIZZLE_API extern __declspec(dllexport) 
#  define DRIZZLE_LOCAL
# endif /* defined(HAVE_VISIBILITY) */
#else  /* defined(BUILDING_LIBDRIZZLE) */
# if defined(_MSC_VER)
#  define DRIZZLE_API extern __declspec(dllimport) 
#  define DRIZZLE_LOCAL
# else
#  define DRIZZLE_API
#  define DRIZZLE_LOCAL
# endif /* defined(_MSC_VER) */
#endif /* defined(BUILDING_LIBDRIZZLE) */
#endif /* _WIN32 */

#endif /* __DRIZZLE_VISIBILITY_H */
