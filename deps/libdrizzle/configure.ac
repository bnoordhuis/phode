# Drizzle Client & Protocol Library
#
# Copyright (C) 2008 Eric Day (eday@oddments.org)
# All rights reserved.
#
# Use and distribution licensed under the BSD license.  See
# the COPYING file in this directory for full text.

AC_PREREQ(2.59)
AC_INIT([libdrizzle],[0.8],[https://launchpad.net/libdrizzle])
AC_CONFIG_SRCDIR([libdrizzle/drizzle.c])
AC_CONFIG_AUX_DIR(config)

PANDORA_CANONICAL_TARGET(ignore-shared-ptr)


LIBDRIZZLE_LIBRARY_VERSION=0:8:0
#                          | | |
#                   +------+ | +---+
#                   |        |     |
#                current:revision:age
#                   |        |     |
#                   |        |     +- increment if interfaces have been added
#                   |        |        set to zero if interfaces have been
#                   |        |        removed or changed
#                   |        +- increment if source code has changed
#                   |           set to zero if current is incremented
#                   +- increment if interfaces have been added, removed or
#                      changed
AC_SUBST(LIBDRIZZLE_LIBRARY_VERSION)

# libdrizzle versioning when linked with GNU ld.
AS_IF([test "$lt_cv_prog_gnu_ld" = "yes"],[
  LD_VERSION_SCRIPT="-Wl,--version-script=\$(top_srcdir)/libdrizzle/libdrizzle.ver"
])
AC_SUBST(LD_VERSION_SCRIPT)

PANDORA_HAVE_LIBSQLITE3

AC_CHECK_FUNC(inet_ntoa, [], [AC_CHECK_LIB(nsl, inet_ntoa)])

AC_CHECK_FUNCS(memchr memmove memset socket strcasecmp strtoul)

AC_CHECK_HEADERS(errno.h fcntl.h netinet/tcp.h stdarg.h stdio.h stdlib.h)
AC_CHECK_HEADERS(string.h sys/uio.h unistd.h)

AC_CONFIG_FILES(Makefile support/libdrizzle.spec support/libdrizzle.pc)

AC_OUTPUT

echo "---"
echo "Configuration summary for $PACKAGE_NAME version $VERSION"
echo ""
echo "   * Installation prefix:       $prefix"
echo "   * System type:               $host_vendor-$host_os"
echo "   * Host CPU:                  $host_cpu"
echo "   * C Compiler:                $CC_VERSION"
echo "   * Assertions enabled:        $ac_cv_assert"
echo "   * Debug enabled:             $with_debug"
echo "   * Profiling enabled:         $ac_profiling"
echo "   * Coverage enabled:          $ac_coverage"
echo "   * Warnings as errors:        $ac_cv_warnings_as_errors"
echo ""
echo "---"
