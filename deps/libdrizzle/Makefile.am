#
# Drizzle Client Library
#
# Copyright (C) 2008 Eric Day (eday@oddments.org)
# All rights reserved.
#
# Use and distribution licensed under the BSD license.  See
# the COPYING file in this directory for full text.
#
#

ACLOCAL_AMFLAGS= -I m4

LDADD= libdrizzle.la

lib_LTLIBRARIES= libdrizzle.la

if HAVE_LIBSQLITE3
SQLITE_PROGS= examples/sqlite_server
endif

bin_PROGRAMS= \
	bin/mysql_password_hash

noinst_PROGRAMS= \
	examples/client \
	examples/simple \
	examples/simple_multi \
	examples/pipe_query \
	examples/server \
	examples/proxy \
	$(SQLITE_PROGS)

libdrizzle_la_CFLAGS= \
	${AM_CFLAGS} \
	-DBUILDING_LIBDRIZZLE

libdrizzle_la_LDFLAGS= \
	$(AM_LDFLAGS) \
	-version-info \
	$(LIBDRIZZLE_LIBRARY_VERSION)

libdrizzle_la_SOURCES= \
	libdrizzle/drizzle.c \
	libdrizzle/conn.c \
	libdrizzle/conn_uds.c \
	libdrizzle/handshake.c \
	libdrizzle/command.c \
	libdrizzle/query.c \
	libdrizzle/result.c \
	libdrizzle/column.c \
	libdrizzle/row.c \
	libdrizzle/field.c \
	libdrizzle/pack.c \
	libdrizzle/state.c \
	libdrizzle/sha1.c

if BUILD_WIN32
libdrizzle_la_SOURCES+= win32/poll.c
endif

examples_server_CFLAGS= $(AM_CFLAGS)
examples_server_SOURCES= examples/server.c

examples_proxy_CFLAGS= $(AM_CFLAGS)
examples_proxy_SOURCES= examples/proxy.c

if HAVE_LIBSQLITE3
examples_sqlite_server_CFLAGS= $(AM_CFLAGS)
examples_sqlite_server_LDFLAGS= libdrizzle.la $(LTLIBSQLITE3)
examples_sqlite_server_DEPENDENCIES= libdrizzle.la
examples_sqlite_server_SOURCES= examples/sqlite_server.c
endif

nobase_include_HEADERS= \
	libdrizzle/drizzle.h \
	libdrizzle/drizzle_client.h \
	libdrizzle/drizzle_server.h \
	libdrizzle/conn.h \
	libdrizzle/conn_client.h \
	libdrizzle/conn_server.h \
	libdrizzle/handshake_client.h \
	libdrizzle/handshake_server.h \
	libdrizzle/command_client.h \
	libdrizzle/command_server.h \
	libdrizzle/query.h \
	libdrizzle/result.h \
	libdrizzle/result_client.h \
	libdrizzle/result_server.h \
	libdrizzle/column.h \
	libdrizzle/column_client.h \
	libdrizzle/column_server.h \
	libdrizzle/row_client.h \
	libdrizzle/row_server.h \
	libdrizzle/field_client.h \
	libdrizzle/field_server.h \
	libdrizzle/constants.h \
	libdrizzle/structs.h \
	libdrizzle/visibility.h

noinst_HEADERS= \
	libdrizzle/common.h \
	libdrizzle/drizzle_local.h \
	libdrizzle/conn_local.h \
	libdrizzle/pack.h \
	libdrizzle/state.h \
	libdrizzle/sha1.h \
	tests/common.h

doxygen:
	${DOXYGEN} docs/Doxyfile.api
	${DOXYGEN} docs/Doxyfile.dev

CLEANFILES= \
	doxerr.log \
	libdrizzle/drizzle_client.h.gch \
	libdrizzle/drizzle_server.h.gch \
	libdrizzle/common.h.gch

MAINTAINERCLEANFILES = \
	m4/libtool.m4 \
	m4/ltoptions.m4 \
	m4/ltsugar.m4 \
	m4/ltversion.m4 \
	m4/lt~obsolete.m4

clean-local: cleandocs
.PHONY: cleandocs
cleandocs:
	-rm -rf docs/api docs/dev

EXTRA_DIST= \
	PROTOCOL \
	config/config.rpath \
	support/libdrizzle.spec \
	docs/Doxyfile.api \
	docs/Doxyfile.dev \
	docs/dev_header.html \
	docs/api_header.html \
	docs/doxygen.h \
	tests/run.sh \
	prototest/drizzle_prototest \
	prototest/drizzle_flood \
	prototest/prototest/__init__.py \
	prototest/prototest/mysql/__init__.py \
	prototest/prototest/mysql/bitfield.py \
	prototest/prototest/mysql/column.py \
	prototest/prototest/mysql/command.py \
	prototest/prototest/mysql/handshake.py \
	prototest/prototest/mysql/packet.py \
	prototest/prototest/mysql/result.py

pkgconfigdir= $(libdir)/pkgconfig
pkgconfig_DATA= support/libdrizzle.pc

test: check

check-valgrind:
	LIBDRIZZLE_TEST_PREFIX="valgrind -q --leak-check=full --show-reachable=yes" make check

check-gdb:
	LIBDRIZZLE_TEST_PREFIX="gdb -q" make check

check-gdb-run:
	LIBDRIZZLE_TEST_PREFIX="gdb -q -x gdb-run" make check

check-time:
	LIBDRIZZLE_TEST_PREFIX="time" make check

check-strace:
	LIBDRIZZLE_TEST_PREFIX="strace -c" make check

check-truss:
	LIBDRIZZLE_TEST_PREFIX="truss -c" make check

TESTS_ENVIRONMENT= ${top_srcdir}/tests/run.sh

check_PROGRAMS= \
	tests/drizzle_st \
	tests/drizzle_con_st \
	tests/drizzle_query_st \
	tests/drizzle_result_st \
	tests/drizzle_column_st \
	tests/client_server

TESTS= $(check_PROGRAMS)

lcov: lcov-clean check
	@echo
	@echo "------------------------------------------------------"
	@echo "Make sure ./configure was run with '--enable-coverage'"
	@echo "------------------------------------------------------"
	@echo
	cd libdrizzle && lcov --capture --directory . --base-directory .. --output-file lcov.out
	genhtml -o lcov -t libdrizzle libdrizzle/lcov.out

lcov-clean: clean
	rm -rf lcov */lcov.out */*.gcda */*.gcno
