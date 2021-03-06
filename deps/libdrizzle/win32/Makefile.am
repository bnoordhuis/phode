## DO NOT EDIT! GENERATED AUTOMATICALLY!
## Process this file with automake to produce Makefile.in.
# Copyright (C) 2002-2008 Free Software Foundation, Inc.
#
# This file is free software, distributed under the terms of the GNU
# General Public License.  As a special exception to the GNU General
# Public License, this file may be distributed as part of a program
# that contains a configuration script generated by Autoconf, under
# the same distribution terms as the rest of that program.
#
# Generated by gnulib-tool.
# Reproduce by: gnulib-tool --import --dir=. --lib=libgnu --source-base=gnulib --m4-base=m4 --doc-base=doc --tests-base=tests --aux-dir=config --lgpl --libtool --macro-prefix=gl poll

AUTOMAKE_OPTIONS = 1.5 gnits

SUBDIRS =
noinst_HEADERS =
noinst_LIBRARIES =
noinst_LTLIBRARIES =
EXTRA_DIST =
BUILT_SOURCES =
SUFFIXES =
MOSTLYCLEANFILES = core *.stackdump
MOSTLYCLEANDIRS =
CLEANFILES =
DISTCLEANFILES =
MAINTAINERCLEANFILES =

AM_CPPFLAGS =

noinst_LTLIBRARIES += libgnu.la

libgnu_la_SOURCES =
libgnu_la_LIBADD = $(gl_LTLIBOBJS)
libgnu_la_DEPENDENCIES = $(gl_LTLIBOBJS)
EXTRA_libgnu_la_SOURCES =
libgnu_la_LDFLAGS = $(AM_LDFLAGS)

## begin gnulib module alloca


EXTRA_DIST += alloca.c

EXTRA_libgnu_la_SOURCES += alloca.c

libgnu_la_LIBADD += @LTALLOCA@
libgnu_la_DEPENDENCIES += @LTALLOCA@
## end   gnulib module alloca

## begin gnulib module alloca-opt

BUILT_SOURCES += $(ALLOCA_H)

# We need the following in order to create <alloca.h> when the system
# doesn't have one that works with the given compiler.
alloca.h: alloca.in.h
	{ echo '/* DO NOT EDIT! GENERATED AUTOMATICALLY! */'; \
	  cat $(srcdir)/alloca.in.h; \
	} > $@-t
	mv -f $@-t $@
MOSTLYCLEANFILES += alloca.h alloca.h-t

EXTRA_DIST += alloca.in.h

## end   gnulib module alloca-opt

## begin gnulib module errno

BUILT_SOURCES += $(ERRNO_H)

# We need the following in order to create <errno.h> when the system
# doesn't have one that is POSIX compliant.
errno.h: errno.in.h
	rm -f $@-t $@
	{ echo '/* DO NOT EDIT! GENERATED AUTOMATICALLY! */' && \
	  sed -e 's|@''INCLUDE_NEXT''@|$(INCLUDE_NEXT)|g' \
	      -e 's|@''PRAGMA_SYSTEM_HEADER''@|@PRAGMA_SYSTEM_HEADER@|g' \
	      -e 's|@''NEXT_ERRNO_H''@|$(NEXT_ERRNO_H)|g' \
	      -e 's|@''EMULTIHOP_HIDDEN''@|$(EMULTIHOP_HIDDEN)|g' \
	      -e 's|@''EMULTIHOP_VALUE''@|$(EMULTIHOP_VALUE)|g' \
	      -e 's|@''ENOLINK_HIDDEN''@|$(ENOLINK_HIDDEN)|g' \
	      -e 's|@''ENOLINK_VALUE''@|$(ENOLINK_VALUE)|g' \
	      -e 's|@''EOVERFLOW_HIDDEN''@|$(EOVERFLOW_HIDDEN)|g' \
	      -e 's|@''EOVERFLOW_VALUE''@|$(EOVERFLOW_VALUE)|g' \
	      < $(srcdir)/errno.in.h; \
	} > $@-t
	mv $@-t $@
MOSTLYCLEANFILES += errno.h errno.h-t

EXTRA_DIST += errno.in.h

## end   gnulib module errno

## begin gnulib module link-warning

LINK_WARNING_H=$(top_srcdir)/config/link-warning.h

EXTRA_DIST += $(top_srcdir)/config/link-warning.h

## end   gnulib module link-warning

## begin gnulib module poll

BUILT_SOURCES += $(POLL_H)

# We need the following in order to create <poll.h> when the system
# doesn't have one.
poll.h: poll.in.h
	{ echo '/* DO NOT EDIT! GENERATED AUTOMATICALLY! */'; \
	  cat $(srcdir)/poll.in.h; \
	} > $@-t
	mv -f $@-t $@
MOSTLYCLEANFILES += poll.h poll.h-t

EXTRA_DIST += poll.c poll.in.h

EXTRA_libgnu_la_SOURCES += poll.c

## end   gnulib module poll

## begin gnulib module sys_select

BUILT_SOURCES += $(SYS_SELECT_H)

# We need the following in order to create <sys/select.h> when the system
# doesn't have one that works with the given compiler.
sys/select.h: sys_select.in.h
	@MKDIR_P@ sys
	rm -f $@-t $@
	{ echo '/* DO NOT EDIT! GENERATED AUTOMATICALLY! */'; \
	  sed -e 's|@''INCLUDE_NEXT''@|$(INCLUDE_NEXT)|g' \
	      -e 's|@''PRAGMA_SYSTEM_HEADER''@|@PRAGMA_SYSTEM_HEADER@|g' \
	      -e 's|@''NEXT_SYS_SELECT_H''@|$(NEXT_SYS_SELECT_H)|g' \
	      -e 's|@''HAVE_SYS_SELECT_H''@|$(HAVE_SYS_SELECT_H)|g' \
	      -e 's|@''GNULIB_SELECT''@|$(GNULIB_SELECT)|g' \
	      -e 's|@''HAVE_WINSOCK2_H''@|$(HAVE_WINSOCK2_H)|g' \
	      -e '/definition of GL_LINK_WARNING/r $(LINK_WARNING_H)' \
	      < $(srcdir)/sys_select.in.h; \
	} > $@-t
	mv $@-t $@
MOSTLYCLEANFILES += sys/select.h sys/select.h-t
MOSTLYCLEANDIRS += sys

EXTRA_DIST += sys_select.in.h

## end   gnulib module sys_select

## begin gnulib module sys_socket

BUILT_SOURCES += $(SYS_SOCKET_H)

# We need the following in order to create <sys/socket.h> when the system
# doesn't have one that works with the given compiler.
sys/socket.h: sys_socket.in.h
	@MKDIR_P@ sys
	{ echo '/* DO NOT EDIT! GENERATED AUTOMATICALLY! */'; \
	  sed -e 's|@''INCLUDE_NEXT''@|$(INCLUDE_NEXT)|g' \
	      -e 's|@''PRAGMA_SYSTEM_HEADER''@|@PRAGMA_SYSTEM_HEADER@|g' \
	      -e 's|@''NEXT_SYS_SOCKET_H''@|$(NEXT_SYS_SOCKET_H)|g' \
	      -e 's|@''HAVE_SYS_SOCKET_H''@|$(HAVE_SYS_SOCKET_H)|g' \
	      -e 's|@''GNULIB_CLOSE''@|$(GNULIB_CLOSE)|g' \
	      -e 's|@''GNULIB_SOCKET''@|$(GNULIB_SOCKET)|g' \
	      -e 's|@''GNULIB_CONNECT''@|$(GNULIB_CONNECT)|g' \
	      -e 's|@''GNULIB_ACCEPT''@|$(GNULIB_ACCEPT)|g' \
	      -e 's|@''GNULIB_BIND''@|$(GNULIB_BIND)|g' \
	      -e 's|@''GNULIB_GETPEERNAME''@|$(GNULIB_GETPEERNAME)|g' \
	      -e 's|@''GNULIB_GETSOCKNAME''@|$(GNULIB_GETSOCKNAME)|g' \
	      -e 's|@''GNULIB_GETSOCKOPT''@|$(GNULIB_GETSOCKOPT)|g' \
	      -e 's|@''GNULIB_LISTEN''@|$(GNULIB_LISTEN)|g' \
	      -e 's|@''GNULIB_RECV''@|$(GNULIB_RECV)|g' \
	      -e 's|@''GNULIB_SEND''@|$(GNULIB_SEND)|g' \
	      -e 's|@''GNULIB_RECVFROM''@|$(GNULIB_RECVFROM)|g' \
	      -e 's|@''GNULIB_SENDTO''@|$(GNULIB_SENDTO)|g' \
	      -e 's|@''GNULIB_SETSOCKOPT''@|$(GNULIB_SETSOCKOPT)|g' \
	      -e 's|@''GNULIB_SHUTDOWN''@|$(GNULIB_SHUTDOWN)|g' \
	      -e 's|@''HAVE_WINSOCK2_H''@|$(HAVE_WINSOCK2_H)|g' \
	      -e 's|@''HAVE_WS2TCPIP_H''@|$(HAVE_WS2TCPIP_H)|g' \
	      -e '/definition of GL_LINK_WARNING/r $(LINK_WARNING_H)' \
	      < $(srcdir)/sys_socket.in.h; \
	} > $@-t
	mv -f $@-t $@
MOSTLYCLEANFILES += sys/socket.h sys/socket.h-t
MOSTLYCLEANDIRS += sys

EXTRA_DIST += sys_socket.in.h

## end   gnulib module sys_socket

## begin gnulib module sys_time

BUILT_SOURCES += $(SYS_TIME_H)

# We need the following in order to create <sys/time.h> when the system
# doesn't have one that works with the given compiler.
sys/time.h: sys_time.in.h
	@MKDIR_P@ sys
	rm -f $@-t $@
	{ echo '/* DO NOT EDIT! GENERATED AUTOMATICALLY! */'; \
	  sed -e 's/@''HAVE_SYS_TIME_H''@/$(HAVE_SYS_TIME_H)/g' \
	      -e 's|@''INCLUDE_NEXT''@|$(INCLUDE_NEXT)|g' \
	      -e 's|@''PRAGMA_SYSTEM_HEADER''@|@PRAGMA_SYSTEM_HEADER@|g' \
	      -e 's|@''NEXT_SYS_TIME_H''@|$(NEXT_SYS_TIME_H)|g' \
	      -e 's/@''REPLACE_GETTIMEOFDAY''@/$(REPLACE_GETTIMEOFDAY)/g' \
	      -e 's/@''HAVE_STRUCT_TIMEVAL''@/$(HAVE_STRUCT_TIMEVAL)/g' \
	      < $(srcdir)/sys_time.in.h; \
	} > $@-t
	mv $@-t $@
MOSTLYCLEANFILES += sys/time.h sys/time.h-t

EXTRA_DIST += sys_time.in.h

## end   gnulib module sys_time

## begin gnulib module dummy

libgnu_la_SOURCES += dummy.c

## end   gnulib module dummy


mostlyclean-local: mostlyclean-generic
	@for dir in '' $(MOSTLYCLEANDIRS); do \
	  if test -n "$$dir" && test -d $$dir; then \
	    echo "rmdir $$dir"; rmdir $$dir; \
	  fi; \
	done; \
	:
