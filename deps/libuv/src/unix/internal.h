/* Copyright Joyent, Inc. and other Node contributors. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef UV_UNIX_INTERNAL_H_
#define UV_UNIX_INTERNAL_H_

#include "uv-common.h"
#include "uv-eio.h"

/* flags */
enum {
  UV_CLOSING  = 0x00000001, /* uv_close() called but not finished. */
  UV_CLOSED   = 0x00000002, /* close(2) finished. */
  UV_READING  = 0x00000004, /* uv_read_start() called. */
  UV_SHUTTING = 0x00000008, /* uv_shutdown() called but not complete. */
  UV_SHUT     = 0x00000010, /* Write side closed. */
  UV_READABLE = 0x00000020, /* The stream is readable */
  UV_WRITABLE = 0x00000040  /* The stream is writable */
};

size_t uv__strlcpy(char* dst, const char* src, size_t size);

int uv__close(int fd);
void uv__req_init(uv_req_t*);
void uv__handle_init(uv_loop_t* loop, uv_handle_t* handle, uv_handle_type type);


int uv__nonblock(int fd, int set) __attribute__((unused));
int uv__cloexec(int fd, int set) __attribute__((unused));
int uv__socket(int domain, int type, int protocol);

/* error */
uv_err_t uv_err_new(uv_loop_t* loop, int sys_error);
uv_err_t uv_err_new_artificial(uv_loop_t* loop, int code);
void uv_fatal_error(const int errorno, const char* syscall);

/* stream */
int uv__stream_open(uv_stream_t*, int fd, int flags);
void uv__stream_io(EV_P_ ev_io* watcher, int revents);
void uv__server_io(EV_P_ ev_io* watcher, int revents);
int uv__accept(int sockfd, struct sockaddr* saddr, socklen_t len);
int uv__connect(uv_connect_t* req, uv_stream_t* stream, struct sockaddr* addr,
    socklen_t addrlen, uv_connect_cb cb);

/* tcp */
int uv_tcp_listen(uv_tcp_t* tcp, int backlog, uv_connection_cb cb);

/* pipe */
int uv_pipe_listen(uv_pipe_t* handle, int backlog, uv_connection_cb cb);
void uv__pipe_accept(EV_P_ ev_io* watcher, int revents);
int uv_pipe_cleanup(uv_pipe_t* handle);

/* udp */
void uv__udp_destroy(uv_udp_t* handle);
void uv__udp_watcher_stop(uv_udp_t* handle, ev_io* w);

#endif /* UV_UNIX_INTERNAL_H_ */
