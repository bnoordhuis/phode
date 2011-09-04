/* Copyright Joyent, Inc. and other Node contributors. All rights reserved.
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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE /* O_CLOEXEC, accept4(), etc. */
#endif

#include "uv.h"
#include "unix/internal.h"

#include <stddef.h> /* NULL */
#include <stdio.h> /* printf */
#include <stdlib.h>
#include <string.h> /* strerror */
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <limits.h> /* PATH_MAX */
#include <sys/uio.h> /* writev */

#if defined(__linux__)

#include <linux/version.h>
#include <features.h>

#undef HAVE_PIPE2
#undef HAVE_ACCEPT4

/* pipe2() requires linux >= 2.6.27 and glibc >= 2.9 */
#if LINUX_VERSION_CODE >= 0x2061B && __GLIBC_PREREQ(2, 9)
#define HAVE_PIPE2
#endif

/* accept4() requires linux >= 2.6.28 and glib >= 2.10 */
#if LINUX_VERSION_CODE >= 0x2061C && __GLIBC_PREREQ(2, 10)
#define HAVE_ACCEPT4
#endif

#endif /* __linux__ */

#ifdef __sun
# include <sys/types.h>
# include <sys/wait.h>
#endif

#if defined(__APPLE__)
#include <mach-o/dyld.h> /* _NSGetExecutablePath */
#endif

#if defined(__FreeBSD__)
#include <sys/sysctl.h>
#include <sys/wait.h>
#endif

static uv_loop_t default_loop_struct;
static uv_loop_t* default_loop_ptr;

void uv__next(EV_P_ ev_idle* watcher, int revents);
static void uv__finish_close(uv_handle_t* handle);



#ifndef __GNUC__
#define __attribute__(a)
#endif


void uv_init() {
  default_loop_ptr = &default_loop_struct;
#if defined(__MAC_OS_X_VERSION_MIN_REQUIRED) && __MAC_OS_X_VERSION_MIN_REQUIRED >= 1060
  default_loop_struct.ev = ev_default_loop(EVBACKEND_KQUEUE);
#else
  default_loop_struct.ev = ev_default_loop(EVFLAG_AUTO);
#endif
  ev_set_userdata(default_loop_struct.ev, default_loop_ptr);
}


void uv_close(uv_handle_t* handle, uv_close_cb close_cb) {
  uv_udp_t* udp;
  uv_async_t* async;
  uv_timer_t* timer;
  uv_stream_t* stream;
  uv_process_t* process;

  handle->close_cb = close_cb;

  switch (handle->type) {
    case UV_NAMED_PIPE:
      uv_pipe_cleanup((uv_pipe_t*)handle);
      /* Fall through. */

    case UV_TCP:
      stream = (uv_stream_t*)handle;

      uv_read_stop(stream);
      ev_io_stop(stream->loop->ev, &stream->write_watcher);

      uv__close(stream->fd);
      stream->fd = -1;

      if (stream->accepted_fd >= 0) {
        uv__close(stream->accepted_fd);
        stream->accepted_fd = -1;
      }

      assert(!ev_is_active(&stream->read_watcher));
      assert(!ev_is_active(&stream->write_watcher));
      break;

    case UV_UDP:
      udp = (uv_udp_t*)handle;
      uv__udp_watcher_stop(udp, &udp->read_watcher);
      uv__udp_watcher_stop(udp, &udp->write_watcher);
      uv__close(udp->fd);
      udp->fd = -1;
      break;

    case UV_PREPARE:
      uv_prepare_stop((uv_prepare_t*) handle);
      break;

    case UV_CHECK:
      uv_check_stop((uv_check_t*) handle);
      break;

    case UV_IDLE:
      uv_idle_stop((uv_idle_t*) handle);
      break;

    case UV_ASYNC:
      async = (uv_async_t*)handle;
      ev_async_stop(async->loop->ev, &async->async_watcher);
      ev_ref(async->loop->ev);
      break;

    case UV_TIMER:
      timer = (uv_timer_t*)handle;
      if (ev_is_active(&timer->timer_watcher)) {
        ev_ref(timer->loop->ev);
      }
      ev_timer_stop(timer->loop->ev, &timer->timer_watcher);
      break;

    case UV_PROCESS:
      process = (uv_process_t*)handle;
      ev_child_stop(process->loop->ev, &process->child_watcher);
      break;

    default:
      assert(0);
  }

  handle->flags |= UV_CLOSING;

  /* This is used to call the on_close callback in the next loop. */
  ev_idle_start(handle->loop->ev, &handle->next_watcher);
  ev_feed_event(handle->loop->ev, &handle->next_watcher, EV_IDLE);
  assert(ev_is_pending(&handle->next_watcher));
}


uv_loop_t* uv_loop_new() {
  uv_loop_t* loop = calloc(1, sizeof(uv_loop_t));
  loop->ev = ev_loop_new(0);
  ev_set_userdata(loop->ev, loop);
  return loop;
}


void uv_loop_delete(uv_loop_t* loop) {
  uv_ares_destroy(loop, loop->channel);
  ev_loop_destroy(loop->ev);
  free(loop);
}


uv_loop_t* uv_default_loop() {
  assert(default_loop_ptr->ev == EV_DEFAULT_UC);
  return default_loop_ptr;
}


int uv_run(uv_loop_t* loop) {
  ev_run(loop->ev, 0);
  return 0;
}


void uv__handle_init(uv_loop_t* loop, uv_handle_t* handle,
    uv_handle_type type) {
  loop->counters.handle_init++;

  handle->loop = loop;
  handle->type = type;
  handle->flags = 0;

  ev_init(&handle->next_watcher, uv__next);
  handle->next_watcher.data = handle;

  /* Ref the loop until this handle is closed. See uv__finish_close. */
  ev_ref(loop->ev);
}


void uv__finish_close(uv_handle_t* handle) {
  uv_loop_t* loop = handle->loop;

  assert(handle->flags & UV_CLOSING);
  assert(!(handle->flags & UV_CLOSED));
  handle->flags |= UV_CLOSED;

  switch (handle->type) {
    case UV_PREPARE:
      assert(!ev_is_active(&((uv_prepare_t*)handle)->prepare_watcher));
      break;

    case UV_CHECK:
      assert(!ev_is_active(&((uv_check_t*)handle)->check_watcher));
      break;

    case UV_IDLE:
      assert(!ev_is_active(&((uv_idle_t*)handle)->idle_watcher));
      break;

    case UV_ASYNC:
      assert(!ev_is_active(&((uv_async_t*)handle)->async_watcher));
      break;

    case UV_TIMER:
      assert(!ev_is_active(&((uv_timer_t*)handle)->timer_watcher));
      break;

    case UV_NAMED_PIPE:
    case UV_TCP:
      assert(!ev_is_active(&((uv_stream_t*)handle)->read_watcher));
      assert(!ev_is_active(&((uv_stream_t*)handle)->write_watcher));
      break;

    case UV_UDP:
      assert(!ev_is_active(&((uv_udp_t*)handle)->read_watcher));
      assert(!ev_is_active(&((uv_udp_t*)handle)->write_watcher));
      assert(((uv_udp_t*)handle)->fd == -1);
      uv__udp_destroy((uv_udp_t*)handle);
      break;

    case UV_PROCESS:
      assert(!ev_is_active(&((uv_process_t*)handle)->child_watcher));
      break;

    default:
      assert(0);
      break;
  }

  ev_idle_stop(loop->ev, &handle->next_watcher);

  if (handle->close_cb) {
    handle->close_cb(handle);
  }

  ev_unref(loop->ev);
}


void uv__next(EV_P_ ev_idle* watcher, int revents) {
  uv_handle_t* handle = watcher->data;
  assert(watcher == &handle->next_watcher);
  assert(revents == EV_IDLE);

  /* For now this function is only to handle the closing event, but we might
   * put more stuff here later.
   */
  assert(handle->flags & UV_CLOSING);
  uv__finish_close(handle);
}


void uv_ref(uv_loop_t* loop) {
  ev_ref(loop->ev);
}


void uv_unref(uv_loop_t* loop) {
  ev_unref(loop->ev);
}


void uv_update_time(uv_loop_t* loop) {
  ev_now_update(loop->ev);
}


int64_t uv_now(uv_loop_t* loop) {
  return (int64_t)(ev_now(loop->ev) * 1000);
}


void uv__req_init(uv_req_t* req) {
  /* loop->counters.req_init++; */
  req->type = UV_UNKNOWN_REQ;
  req->data = NULL;
}


static void uv__prepare(EV_P_ ev_prepare* w, int revents) {
  uv_prepare_t* prepare = w->data;

  if (prepare->prepare_cb) {
    prepare->prepare_cb(prepare, 0);
  }
}


int uv_prepare_init(uv_loop_t* loop, uv_prepare_t* prepare) {
  uv__handle_init(loop, (uv_handle_t*)prepare, UV_PREPARE);
  loop->counters.prepare_init++;

  ev_prepare_init(&prepare->prepare_watcher, uv__prepare);
  prepare->prepare_watcher.data = prepare;

  prepare->prepare_cb = NULL;

  return 0;
}


int uv_prepare_start(uv_prepare_t* prepare, uv_prepare_cb cb) {
  int was_active = ev_is_active(&prepare->prepare_watcher);

  prepare->prepare_cb = cb;

  ev_prepare_start(prepare->loop->ev, &prepare->prepare_watcher);

  if (!was_active) {
    ev_unref(prepare->loop->ev);
  }

  return 0;
}


int uv_prepare_stop(uv_prepare_t* prepare) {
  int was_active = ev_is_active(&prepare->prepare_watcher);

  ev_prepare_stop(prepare->loop->ev, &prepare->prepare_watcher);

  if (was_active) {
    ev_ref(prepare->loop->ev);
  }
  return 0;
}



static void uv__check(EV_P_ ev_check* w, int revents) {
  uv_check_t* check = w->data;

  if (check->check_cb) {
    check->check_cb(check, 0);
  }
}


int uv_check_init(uv_loop_t* loop, uv_check_t* check) {
  uv__handle_init(loop, (uv_handle_t*)check, UV_CHECK);
  loop->counters.check_init++;

  ev_check_init(&check->check_watcher, uv__check);
  check->check_watcher.data = check;

  check->check_cb = NULL;

  return 0;
}


int uv_check_start(uv_check_t* check, uv_check_cb cb) {
  int was_active = ev_is_active(&check->check_watcher);

  check->check_cb = cb;

  ev_check_start(check->loop->ev, &check->check_watcher);

  if (!was_active) {
    ev_unref(check->loop->ev);
  }

  return 0;
}


int uv_check_stop(uv_check_t* check) {
  int was_active = ev_is_active(&check->check_watcher);

  ev_check_stop(check->loop->ev, &check->check_watcher);

  if (was_active) {
    ev_ref(check->loop->ev);
  }

  return 0;
}


static void uv__idle(EV_P_ ev_idle* w, int revents) {
  uv_idle_t* idle = (uv_idle_t*)(w->data);

  if (idle->idle_cb) {
    idle->idle_cb(idle, 0);
  }
}



int uv_idle_init(uv_loop_t* loop, uv_idle_t* idle) {
  uv__handle_init(loop, (uv_handle_t*)idle, UV_IDLE);
  loop->counters.idle_init++;

  ev_idle_init(&idle->idle_watcher, uv__idle);
  idle->idle_watcher.data = idle;

  idle->idle_cb = NULL;

  return 0;
}


int uv_idle_start(uv_idle_t* idle, uv_idle_cb cb) {
  int was_active = ev_is_active(&idle->idle_watcher);

  idle->idle_cb = cb;
  ev_idle_start(idle->loop->ev, &idle->idle_watcher);

  if (!was_active) {
    ev_unref(idle->loop->ev);
  }

  return 0;
}


int uv_idle_stop(uv_idle_t* idle) {
  int was_active = ev_is_active(&idle->idle_watcher);

  ev_idle_stop(idle->loop->ev, &idle->idle_watcher);

  if (was_active) {
    ev_ref(idle->loop->ev);
  }

  return 0;
}


int uv_is_active(uv_handle_t* handle) {
  switch (handle->type) {
    case UV_TIMER:
      return ev_is_active(&((uv_timer_t*)handle)->timer_watcher);

    case UV_PREPARE:
      return ev_is_active(&((uv_prepare_t*)handle)->prepare_watcher);

    case UV_CHECK:
      return ev_is_active(&((uv_check_t*)handle)->check_watcher);

    case UV_IDLE:
      return ev_is_active(&((uv_idle_t*)handle)->idle_watcher);

    default:
      return 1;
  }
}


static void uv__async(EV_P_ ev_async* w, int revents) {
  uv_async_t* async = w->data;

  if (async->async_cb) {
    async->async_cb(async, 0);
  }
}


int uv_async_init(uv_loop_t* loop, uv_async_t* async, uv_async_cb async_cb) {
  uv__handle_init(loop, (uv_handle_t*)async, UV_ASYNC);
  loop->counters.async_init++;

  ev_async_init(&async->async_watcher, uv__async);
  async->async_watcher.data = async;

  async->async_cb = async_cb;

  /* Note: This does not have symmetry with the other libev wrappers. */
  ev_async_start(loop->ev, &async->async_watcher);
  ev_unref(loop->ev);

  return 0;
}


int uv_async_send(uv_async_t* async) {
  ev_async_send(async->loop->ev, &async->async_watcher);
  return 0;
}


static void uv__timer_cb(EV_P_ ev_timer* w, int revents) {
  uv_timer_t* timer = w->data;

  if (!ev_is_active(w)) {
    ev_ref(EV_A);
  }

  if (timer->timer_cb) {
    timer->timer_cb(timer, 0);
  }
}


int uv_timer_init(uv_loop_t* loop, uv_timer_t* timer) {
  uv__handle_init(loop, (uv_handle_t*)timer, UV_TIMER);
  loop->counters.timer_init++;

  ev_init(&timer->timer_watcher, uv__timer_cb);
  timer->timer_watcher.data = timer;

  return 0;
}


int uv_timer_start(uv_timer_t* timer, uv_timer_cb cb, int64_t timeout,
    int64_t repeat) {
  if (ev_is_active(&timer->timer_watcher)) {
    return -1;
  }

  timer->timer_cb = cb;
  ev_timer_set(&timer->timer_watcher, timeout / 1000.0, repeat / 1000.0);
  ev_timer_start(timer->loop->ev, &timer->timer_watcher);
  ev_unref(timer->loop->ev);
  return 0;
}


int uv_timer_stop(uv_timer_t* timer) {
  if (ev_is_active(&timer->timer_watcher)) {
    ev_ref(timer->loop->ev);
  }

  ev_timer_stop(timer->loop->ev, &timer->timer_watcher);
  return 0;
}


int uv_timer_again(uv_timer_t* timer) {
  if (!ev_is_active(&timer->timer_watcher)) {
    uv_err_new(timer->loop, EINVAL);
    return -1;
  }

  ev_timer_again(timer->loop->ev, &timer->timer_watcher);
  return 0;
}

void uv_timer_set_repeat(uv_timer_t* timer, int64_t repeat) {
  assert(timer->type == UV_TIMER);
  timer->timer_watcher.repeat = repeat / 1000.0;
}

int64_t uv_timer_get_repeat(uv_timer_t* timer) {
  assert(timer->type == UV_TIMER);
  return (int64_t)(1000 * timer->timer_watcher.repeat);
}


static int uv_getaddrinfo_done(eio_req* req) {
  uv_getaddrinfo_t* handle = req->data;

  uv_unref(handle->loop);

  free(handle->hints);
  free(handle->service);
  free(handle->hostname);

  if (handle->retcode != 0) {
    /* TODO how to display gai error strings? */
    uv_err_new(handle->loop, handle->retcode);
  }

  handle->cb(handle, handle->retcode, handle->res);

  freeaddrinfo(handle->res);
  handle->res = NULL;

  return 0;
}


static void getaddrinfo_thread_proc(eio_req *req) {
  uv_getaddrinfo_t* handle = req->data;

  handle->retcode = getaddrinfo(handle->hostname,
                                handle->service,
                                handle->hints,
                                &handle->res);
}


/* stub implementation of uv_getaddrinfo */
int uv_getaddrinfo(uv_loop_t* loop, 
                   uv_getaddrinfo_t* handle,
                   uv_getaddrinfo_cb cb,
                   const char* hostname,
                   const char* service,
                   const struct addrinfo* hints) {
  eio_req* req;
  uv_eio_init(loop);

  if (handle == NULL || cb == NULL ||
      (hostname == NULL && service == NULL)) {
    uv_err_new_artificial(loop, UV_EINVAL);
    return -1;
  }

  memset(handle, 0, sizeof(uv_getaddrinfo_t));

  /* TODO don't alloc so much. */

  if (hints) {
    handle->hints = malloc(sizeof(struct addrinfo));
    memcpy(&handle->hints, hints, sizeof(struct addrinfo));
  }

  /* TODO security! check lengths, check return values. */

  handle->loop = loop;
  handle->cb = cb;
  handle->hostname = hostname ? strdup(hostname) : NULL;
  handle->service = service ? strdup(service) : NULL;

  /* TODO check handle->hostname == NULL */
  /* TODO check handle->service == NULL */

  uv_ref(loop);

  req = eio_custom(getaddrinfo_thread_proc, EIO_PRI_DEFAULT,
      uv_getaddrinfo_done, handle);
  assert(req);
  assert(req->data == handle);

  return 0;
}


/* Open a socket in non-blocking close-on-exec mode, atomically if possible. */
int uv__socket(int domain, int type, int protocol) {
#if defined(SOCK_NONBLOCK) && defined(SOCK_CLOEXEC)
  return socket(domain, type | SOCK_NONBLOCK | SOCK_CLOEXEC, protocol);
#else
  int sockfd;

  if ((sockfd = socket(domain, type, protocol)) == -1) {
    return -1;
  }

  if (uv__nonblock(sockfd, 1) == -1 || uv__cloexec(sockfd, 1) == -1) {
    uv__close(sockfd);
    return -1;
  }

  return sockfd;
#endif
}


int uv__accept(int sockfd, struct sockaddr* saddr, socklen_t slen) {
  int peerfd;

  assert(sockfd >= 0);

  do {
#if defined(HAVE_ACCEPT4)
    peerfd = accept4(sockfd, saddr, &slen, SOCK_NONBLOCK | SOCK_CLOEXEC);
#else
    if ((peerfd = accept(sockfd, saddr, &slen)) != -1) {
      if (uv__cloexec(peerfd, 1) == -1 || uv__nonblock(peerfd, 1) == -1) {
        uv__close(peerfd);
        return -1;
      }
    }
#endif
  }
  while (peerfd == -1 && errno == EINTR);

  return peerfd;
}


int uv__close(int fd) {
  int status;

  /*
   * Retry on EINTR. You may think this is academic but on linux
   * and probably other Unices too, close(2) is interruptible.
   * Failing to handle EINTR is a common source of fd leaks.
   */
  do {
    status = close(fd);
  }
  while (status == -1 && errno == EINTR);

  return status;
}


int uv__nonblock(int fd, int set) {
  int flags;

  if ((flags = fcntl(fd, F_GETFL)) == -1) {
    return -1;
  }

  if (set) {
    flags |= O_NONBLOCK;
  } else {
    flags &= ~O_NONBLOCK;
  }

  if (fcntl(fd, F_SETFL, flags) == -1) {
    return -1;
  }

  return 0;
}


int uv__cloexec(int fd, int set) {
  int flags;

  if ((flags = fcntl(fd, F_GETFD)) == -1) {
    return -1;
  }

  if (set) {
    flags |= FD_CLOEXEC;
  } else {
    flags &= ~FD_CLOEXEC;
  }

  if (fcntl(fd, F_SETFD, flags) == -1) {
    return -1;
  }

  return 0;
}


/* TODO move to uv-common.c? */
size_t uv__strlcpy(char* dst, const char* src, size_t size) {
  const char *org;

  if (size == 0) {
    return 0;
  }

  org = src;
  while (size > 1) {
    if ((*dst++ = *src++) == '\0') {
      return org - src;
    }
  }
  *dst = '\0';

  return src - org;
}


uv_stream_t* uv_std_handle(uv_loop_t* loop, uv_std_type type) {
  assert(0 && "implement me");
  return NULL;
}

