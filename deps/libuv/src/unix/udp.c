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

#include "uv.h"
#include "internal.h"

#include <assert.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>


static void uv__udp_watcher_start(uv_udp_t* handle, ev_io* w);
static void uv__udp_run_completed(uv_udp_t* handle);
static void uv__udp_run_pending(uv_udp_t* handle);
static void uv__udp_recvmsg(uv_udp_t* handle);
static void uv__udp_sendmsg(uv_udp_t* handle);
static void uv__udp_io(EV_P_ ev_io* w, int events);
static int uv__udp_bind(uv_udp_t* handle, int domain, struct sockaddr* addr,
    socklen_t len, unsigned flags);
static int uv__udp_maybe_deferred_bind(uv_udp_t* handle, int domain);
static int uv__udp_send(uv_udp_send_t* req, uv_udp_t* handle, uv_buf_t bufs[],
    int bufcnt, struct sockaddr* addr, socklen_t addrlen, uv_udp_send_cb send_cb);


static void uv__udp_watcher_start(uv_udp_t* handle, ev_io* w) {
  int flags;

  assert(w == &handle->read_watcher
      || w == &handle->write_watcher);

  flags = (w == &handle->read_watcher ? EV_READ : EV_WRITE);

  w->data = handle;
  ev_set_cb(w, uv__udp_io);
  ev_io_set(w, handle->fd, flags);
  ev_io_start(handle->loop->ev, w);
}


void uv__udp_watcher_stop(uv_udp_t* handle, ev_io* w) {
  int flags;

  assert(w == &handle->read_watcher
      || w == &handle->write_watcher);

  flags = (w == &handle->read_watcher ? EV_READ : EV_WRITE);

  ev_io_stop(handle->loop->ev, w);
  ev_io_set(w, -1, flags);
  ev_set_cb(w, NULL);
  w->data = (void*)0xDEADBABE;
}


void uv__udp_destroy(uv_udp_t* handle) {
  uv_udp_send_t* req;
  ngx_queue_t* q;

  uv__udp_run_completed(handle);

  while (!ngx_queue_empty(&handle->write_queue)) {
    q = ngx_queue_head(&handle->write_queue);
    ngx_queue_remove(q);

    req = ngx_queue_data(q, uv_udp_send_t, queue);
    if (req->send_cb) {
      /* FIXME proper error code like UV_EABORTED */
      uv_err_new_artificial(handle->loop, UV_EINTR);
      req->send_cb(req, -1);
    }
  }

  /* Now tear down the handle. */
  handle->flags = 0;
  handle->recv_cb = NULL;
  handle->alloc_cb = NULL;
  /* but _do not_ touch close_cb */

  if (handle->fd != -1) {
    uv__close(handle->fd);
    handle->fd = -1;
  }

  uv__udp_watcher_stop(handle, &handle->read_watcher);
  uv__udp_watcher_stop(handle, &handle->write_watcher);
}


static void uv__udp_run_pending(uv_udp_t* handle) {
  uv_udp_send_t* req;
  ngx_queue_t* q;
  struct msghdr h;
  ssize_t size;

  while (!ngx_queue_empty(&handle->write_queue)) {
    q = ngx_queue_head(&handle->write_queue);
    assert(q != NULL);

    req = ngx_queue_data(q, uv_udp_send_t, queue);
    assert(req != NULL);

    memset(&h, 0, sizeof h);
    h.msg_name = &req->addr;
    h.msg_namelen = req->addrlen;
    h.msg_iov = (struct iovec*)req->bufs;
    h.msg_iovlen = req->bufcnt;

    do {
      size = sendmsg(handle->fd, &h, 0);
    }
    while (size == -1 && errno == EINTR);

    /* TODO try to write once or twice more in the
     * hope that the socket becomes readable again?
     */
    if (size == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
      break;

    req->status = (size == -1 ? -errno : size);

#ifndef NDEBUG
    /* Sanity check. */
    if (size != -1) {
      ssize_t nbytes;
      int i;

      for (nbytes = i = 0; i < req->bufcnt; i++)
        nbytes += req->bufs[i].len;

      assert(size == nbytes);
    }
#endif

    /* Sending a datagram is an atomic operation: either all data
     * is written or nothing is (and EMSGSIZE is raised). That is
     * why we don't handle partial writes. Just pop the request
     * off the write queue and onto the completed queue, done.
     */
    ngx_queue_remove(&req->queue);
    ngx_queue_insert_tail(&handle->write_completed_queue, &req->queue);
  }
}


static void uv__udp_run_completed(uv_udp_t* handle) {
  uv_udp_send_t* req;
  ngx_queue_t* q;

  while (!ngx_queue_empty(&handle->write_completed_queue)) {
    q = ngx_queue_head(&handle->write_completed_queue);
    assert(q != NULL);

    ngx_queue_remove(q);

    req = ngx_queue_data(q, uv_udp_send_t, queue);
    assert(req != NULL);

    if (req->bufs != req->bufsml)
      free(req->bufs);

    if (req->send_cb == NULL)
      continue;

    /* req->status >= 0 == bytes written
     * req->status <  0 == errno
     */
    if (req->status >= 0) {
      req->send_cb(req, 0);
    }
    else {
      uv_err_new(handle->loop, -req->status);
      req->send_cb(req, -1);
    }
  }
}


static void uv__udp_recvmsg(uv_udp_t* handle) {
  struct sockaddr_storage peer;
  struct msghdr h;
  ssize_t nread;
  uv_buf_t buf;
  int flags;

  assert(handle->recv_cb != NULL);
  assert(handle->alloc_cb != NULL);

  do {
    /* FIXME: hoist alloc_cb out the loop but for now follow uv__read() */
    buf = handle->alloc_cb((uv_handle_t*)handle, 64 * 1024);
    assert(buf.len > 0);
    assert(buf.base != NULL);

    memset(&h, 0, sizeof h);
    h.msg_name = &peer;
    h.msg_namelen = sizeof peer;
    h.msg_iov = (struct iovec*)&buf;
    h.msg_iovlen = 1;

    do {
      nread = recvmsg(handle->fd, &h, 0);
    }
    while (nread == -1 && errno == EINTR);

    if (nread == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        uv_err_new(handle->loop, EAGAIN);
        handle->recv_cb(handle, 0, buf, NULL, 0);
      }
      else {
        uv_err_new(handle->loop, errno);
        handle->recv_cb(handle, -1, buf, NULL, 0);
      }
    }
    else {
      flags = 0;

      if (h.msg_flags & MSG_TRUNC)
        flags |= UV_UDP_PARTIAL;

      handle->recv_cb(handle,
                      nread,
                      buf,
                      (struct sockaddr*)&peer,
                      flags);
    }
  }
  /* recv_cb callback may decide to pause or close the handle */
  while (nread != -1
      && handle->fd != -1
      && handle->recv_cb != NULL);
}


static void uv__udp_sendmsg(uv_udp_t* handle) {
  assert(!ngx_queue_empty(&handle->write_queue)
      || !ngx_queue_empty(&handle->write_completed_queue));

  /* Write out pending data first. */
  uv__udp_run_pending(handle);

  /* Drain 'request completed' queue. */
  uv__udp_run_completed(handle);

  if (!ngx_queue_empty(&handle->write_completed_queue)) {
    /* Schedule completion callbacks. */
    ev_feed_event(handle->loop->ev, &handle->write_watcher, EV_WRITE);
  }
  else if (ngx_queue_empty(&handle->write_queue)) {
    /* Pending queue and completion queue empty, stop watcher. */
    uv__udp_watcher_stop(handle, &handle->write_watcher);
  }
}


static void uv__udp_io(EV_P_ ev_io* w, int events) {
  uv_udp_t* handle;

  handle = w->data;
  assert(handle != NULL);
  assert(handle->type == UV_UDP);
  assert(handle->fd >= 0);
  assert(!(events & ~(EV_READ|EV_WRITE)));

  if (events & EV_READ)
    uv__udp_recvmsg(handle);

  if (events & EV_WRITE)
    uv__udp_sendmsg(handle);
}


static int uv__udp_bind(uv_udp_t* handle,
                        int domain,
                        struct sockaddr* addr,
                        socklen_t len,
                        unsigned flags) {
  int saved_errno;
  int status;
  int yes;
  int fd;

  saved_errno = errno;
  status = -1;

  /* Check for bad flags. */
  if (flags & ~UV_UDP_IPV6ONLY) {
    uv_err_new(handle->loop, EINVAL);
    goto out;
  }

  /* Cannot set IPv6-only mode on non-IPv6 socket. */
  if ((flags & UV_UDP_IPV6ONLY) && domain != AF_INET6) {
    uv_err_new(handle->loop, EINVAL);
    goto out;
  }

  /* Check for already active socket. */
  if (handle->fd != -1) {
    uv_err_new_artificial(handle->loop, UV_EALREADY);
    goto out;
  }

  if ((fd = uv__socket(domain, SOCK_DGRAM, 0)) == -1) {
    uv_err_new(handle->loop, errno);
    goto out;
  }

  if (flags & UV_UDP_IPV6ONLY) {
#ifdef IPV6_V6ONLY
    yes = 1;
    if (setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &yes, sizeof yes) == -1) {
      uv_err_new(handle->loop, errno);
      goto out;
    }
#else
    uv_err_new((uv_handle_t*)handle, ENOTSUP);
    goto out;
#endif
  }

  if (bind(fd, addr, len) == -1) {
    uv_err_new(handle->loop, errno);
    goto out;
  }

  handle->fd = fd;
  status = 0;

out:
  if (status)
    uv__close(fd);

  errno = saved_errno;
  return status;
}


static int uv__udp_maybe_deferred_bind(uv_udp_t* handle, int domain) {
  struct sockaddr_storage taddr;
  socklen_t addrlen;

  assert(domain == AF_INET || domain == AF_INET6);

  if (handle->fd != -1)
    return 0;

  switch (domain) {
  case AF_INET:
  {
    struct sockaddr_in* addr = (void*)&taddr;
    memset(addr, 0, sizeof *addr);
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = INADDR_ANY;
    addrlen = sizeof *addr;
    break;
  }
  case AF_INET6:
  {
    struct sockaddr_in6* addr = (void*)&taddr;
    memset(addr, 0, sizeof *addr);
    addr->sin6_family = AF_INET6;
    addr->sin6_addr = in6addr_any;
    addrlen = sizeof *addr;
    break;
  }
  default:
    assert(0 && "unsupported address family");
    abort();
  }

  return uv__udp_bind(handle, domain, (struct sockaddr*)&taddr, addrlen, 0);
}


static int uv__udp_send(uv_udp_send_t* req,
                        uv_udp_t* handle,
                        uv_buf_t bufs[],
                        int bufcnt,
                        struct sockaddr* addr,
                        socklen_t addrlen,
                        uv_udp_send_cb send_cb) {
  if (uv__udp_maybe_deferred_bind(handle, addr->sa_family))
    return -1;

  /* Don't use uv__req_init(), it zeroes the data field. */
  handle->loop->counters.req_init++;

  memcpy(&req->addr, addr, addrlen);
  req->addrlen = addrlen;
  req->send_cb = send_cb;
  req->handle = handle;
  req->bufcnt = bufcnt;
  req->type = UV_UDP_SEND;

  if (bufcnt <= UV_REQ_BUFSML_SIZE) {
    req->bufs = req->bufsml;
  }
  else if ((req->bufs = malloc(bufcnt * sizeof(bufs[0]))) == NULL) {
    uv_err_new(handle->loop, ENOMEM);
    return -1;
  }
  memcpy(req->bufs, bufs, bufcnt * sizeof(bufs[0]));

  ngx_queue_insert_tail(&handle->write_queue, &req->queue);
  uv__udp_watcher_start(handle, &handle->write_watcher);

  return 0;
}


int uv_udp_init(uv_loop_t* loop, uv_udp_t* handle) {
  memset(handle, 0, sizeof *handle);

  uv__handle_init(loop, (uv_handle_t*)handle, UV_UDP);
  loop->counters.udp_init++;

  handle->fd = -1;
  ngx_queue_init(&handle->write_queue);
  ngx_queue_init(&handle->write_completed_queue);

  return 0;
}


int uv_udp_bind(uv_udp_t* handle, struct sockaddr_in addr, unsigned flags) {
  return uv__udp_bind(handle,
                      AF_INET,
                      (struct sockaddr*)&addr,
                      sizeof addr,
                      flags);
}


int uv_udp_bind6(uv_udp_t* handle, struct sockaddr_in6 addr, unsigned flags) {
  return uv__udp_bind(handle,
                      AF_INET6,
                      (struct sockaddr*)&addr,
                      sizeof addr,
                      flags);
}


int uv_udp_getsockname(uv_udp_t* handle, struct sockaddr* name,
    int* namelen) {
  socklen_t socklen;
  int saved_errno;
  int rv = 0;

  /* Don't clobber errno. */
  saved_errno = errno;

  if (handle->fd < 0) {
    uv_err_new(handle->loop, EINVAL);
    rv = -1;
    goto out;
  }

  /* sizeof(socklen_t) != sizeof(int) on some systems. */
  socklen = (socklen_t)*namelen;

  if (getsockname(handle->fd, name, &socklen) == -1) {
    uv_err_new(handle->loop, errno);
    rv = -1;
  } else {
    *namelen = (int)socklen;
  }

out:
  errno = saved_errno;
  return rv;
}


int uv_udp_send(uv_udp_send_t* req,
                uv_udp_t* handle,
                uv_buf_t bufs[],
                int bufcnt,
                struct sockaddr_in addr,
                uv_udp_send_cb send_cb) {
  return uv__udp_send(req,
                      handle,
                      bufs,
                      bufcnt,
                      (struct sockaddr*)&addr,
                      sizeof addr,
                      send_cb);
}


int uv_udp_send6(uv_udp_send_t* req,
                 uv_udp_t* handle,
                 uv_buf_t bufs[],
                 int bufcnt,
                 struct sockaddr_in6 addr,
                 uv_udp_send_cb send_cb) {
  return uv__udp_send(req,
                      handle,
                      bufs,
                      bufcnt,
                      (struct sockaddr*)&addr,
                      sizeof addr,
                      send_cb);
}


int uv_udp_recv_start(uv_udp_t* handle,
                      uv_alloc_cb alloc_cb,
                      uv_udp_recv_cb recv_cb) {
  if (alloc_cb == NULL || recv_cb == NULL) {
    uv_err_new_artificial(handle->loop, UV_EINVAL);
    return -1;
  }

  if (ev_is_active(&handle->read_watcher)) {
    uv_err_new_artificial(handle->loop, UV_EALREADY);
    return -1;
  }

  if (uv__udp_maybe_deferred_bind(handle, AF_INET))
    return -1;

  handle->alloc_cb = alloc_cb;
  handle->recv_cb = recv_cb;
  uv__udp_watcher_start(handle, &handle->read_watcher);

  return 0;
}


int uv_udp_recv_stop(uv_udp_t* handle) {
  uv__udp_watcher_stop(handle, &handle->read_watcher);
  handle->alloc_cb = NULL;
  handle->recv_cb = NULL;
  return 0;
}
