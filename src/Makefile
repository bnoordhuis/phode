-include Makefile.local

CC	= gcc
CFLAGS	= $(INCLUDES:%=-I%) -I../deps/libuv/include -Wall -Wextra -Wno-unused-parameter -fPIC -g -O0
LDFLAGS	= 

all:	../node.so

../node.so:	ext.o ../deps/libuv/uv.a
	$(CC) -shared -o $@ $^ $(LDFLAGS)

ext.o:	ext.c

../deps/libuv/uv.a:
	$(MAKE) -C ../deps/libuv CFLAGS="-g -fPIC"
