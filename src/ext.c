/*
 * Copyright (c) 2011, Ben Noordhuis <info@bnoordhuis.nl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "Zend/zend_exceptions.h"

#include "uv.h"

#include <stddef.h> /* offsetof */

#define container_of(ptr, type, member) \
  ((type *) ((char *) (ptr) - offsetof(type, member)))


#ifdef __GNUC__
# define MAYBE_UNUSED __attribute__ ((unused))
#else
# define MAYBE_UNUSED /* TODO */
#endif


/* Oh man! we're fucked. We have to do the same bullshit as php does! */
#ifdef ZTS
# define TSRMLS_SET(o)    (o)->TSRMLS_C = TSRMLS_C
# define TSRMLS_GET(o)    TSRMLS_C = (o)->TSRMLS_C
# define TSRMLS_D_GET(o)  TSRMLS_D = (o)->TSRMLS_C

#else /* ZTS not defined */
# define TSRMLS_SET(o)    /* empty */
# define TSRMLS_GET(o)    /* empty */
# define TSRMLS_D_GET(o)  /* empty */

#endif /* ZTS not defined */


typedef struct {
  /* obj must be the first member, because it must be safe to cast */
  /* tcp_wrap* to zend_object */
  zend_object obj;
  uv_tcp_t handle;
  zval* close_cb;
  zval* connection_cb;
  unsigned dead:1;
  unsigned listening:1;
  TSRMLS_D;
} tcp_wrap_t;


typedef struct {
  uv_connect_t req;
  zval* callback;
  TSRMLS_D;
} connect_wrap_t;


typedef struct {
  uv_write_t req;
  zval* callback;
  zval* string;
  TSRMLS_D;
} write_wrap_t;

zend_class_entry* tcp_ce;


/* Shamelessly nicked from mongo-php-driver */
#if ZEND_MODULE_API_NO >= 20100525
#define init_properties(obj, class_type) \
  object_properties_init((obj), class_type)
#else
#define init_properties(obj, class_type)                      \
  do {                                                        \
    zval *tmp;                                                \
    zend_hash_copy((obj)->properties,                         \
                   &class_type->default_properties,           \
                   (copy_ctor_func_t) zval_add_ref,           \
                   (void *) &tmp,                             \
                   sizeof(zval*));                            \
  }                                                           \
  while (0)
#endif


#define HEALTHCHECK(handle)                                       \
  if ((handle)->dead) {                                           \
    zend_throw_exception(zend_exception_get_default(TSRMLS_C),    \
                         "cannot call methods on a dead handle",  \
                         0                                        \
                         TSRMLS_CC);                              \
    RETURN_NULL();                                                \
  }

#define THROW_ERROR(message)                                      \
  zend_throw_exception(zend_exception_get_default(TSRMLS_C),      \
                       message,                                   \
                       0                                          \
                       TSRMLS_CC);                                \


static void tcp_wrap_free(void *object TSRMLS_DC) {
  tcp_wrap_t *wrap = (tcp_wrap_t*) object;
  zend_object_std_dtor(&wrap->obj TSRMLS_CC);
  efree(wrap);
}


static zend_object_value tcp_new(zend_class_entry *class_type TSRMLS_DC) {
  zend_object_value instance;
  tcp_wrap_t *wrap;

  wrap = (tcp_wrap_t*) emalloc(sizeof *wrap);

  uv_tcp_init(uv_default_loop(), &wrap->handle);

  zend_object_std_init(&wrap->obj, class_type TSRMLS_CC);
  init_properties(&wrap->obj, class_type);

  TSRMLS_SET(wrap);

  wrap->handle.data = (void*) wrap;
  wrap->listening = 0;
  wrap->connection_cb = NULL;

  instance.handle = zend_objects_store_put((void*) wrap,
                                           (zend_objects_store_dtor_t) zend_objects_destroy_object,
                                           tcp_wrap_free,
                                           NULL
                                           TSRMLS_CC);
  instance.handlers = zend_get_std_object_handlers();

  return instance;
}


static void call_callback(zval* callback, int argc, zval* argv[] TSRMLS_DC) {
   zend_fcall_info fci = empty_fcall_info;
   zend_fcall_info_cache fci_cache = empty_fcall_info_cache;
   char *is_callable_error = NULL;
   zval* result;

   if (zend_fcall_info_init(callback, 0, &fci, &fci_cache, NULL, &is_callable_error TSRMLS_CC) == SUCCESS) {
     fci.retval_ptr_ptr = &result;
     fci.param_count = argc;
     fci.params = &argv;
     zend_call_function(&fci, &fci_cache TSRMLS_CC);
   }
}


static void tcp_connect_cb(uv_connect_t* req, int status) {
  connect_wrap_t* wrap = container_of(req, connect_wrap_t, req);
  TSRMLS_D_GET(wrap);

  printf("status: %d\n", status);

  call_callback(wrap->callback, 0, NULL TSRMLS_CC);
  Z_DELREF_P(wrap->callback);
}


PHP_METHOD(TCP, connect) {
  char* ip;
  int ip_length;
  int port;
  zval* callback;
  connect_wrap_t* connect_wrap;
  tcp_wrap_t* tcp_wrap;
  int r;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "slz", &ip, &ip_length, &port, &callback) == FAILURE) {
    return;
  }

  tcp_wrap = (tcp_wrap_t*) zend_object_store_get_object(getThis() TSRMLS_CC);
  HEALTHCHECK(tcp_wrap);

  connect_wrap = (connect_wrap_t*) emalloc(sizeof *connect_wrap);

  r = uv_tcp_connect(&connect_wrap->req, &tcp_wrap->handle, uv_ip4_addr(ip, port), tcp_connect_cb);
  printf("== %d\n", r);

  connect_wrap->callback = callback;
  Z_ADDREF_P(callback);
  TSRMLS_SET(connect_wrap);

  RETURN_NULL();
}


static void tcp_write_cb(uv_write_t* req, int status) {
  write_wrap_t* wrap = container_of(req, write_wrap_t, req);
  TSRMLS_D_GET(wrap);

  printf("write status: %d\n", status);

  call_callback(wrap->callback, 0, NULL TSRMLS_CC);
  Z_DELREF_P(wrap->callback);
  Z_DELREF_P(wrap->string);
}


PHP_METHOD(TCP, write) {
  zval* string;
  zval* callback;
  write_wrap_t* write_wrap;
  tcp_wrap_t* tcp_wrap;
  uv_buf_t buf;
  int r;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &string, &callback) == FAILURE) {
    return;
  }

  tcp_wrap = (tcp_wrap_t*) zend_object_store_get_object(getThis() TSRMLS_CC);
  HEALTHCHECK(tcp_wrap);

  write_wrap = (write_wrap_t*) emalloc(sizeof *write_wrap);

  /* Todo: leverage php's COW feaure */
  buf.base = Z_STRVAL_P(string);
  buf.len = Z_STRLEN_P(string);

  r = uv_write(&write_wrap->req, (uv_stream_t*) &tcp_wrap->handle, &buf, 1, tcp_write_cb);
  printf("== %d\n", r);

  write_wrap->callback = callback;
  Z_ADDREF_P(callback);
  write_wrap->string = string;
  Z_ADDREF_P(string);
  TSRMLS_SET(write_wrap);

  RETURN_NULL();
}


static void tcp_close_cb(uv_handle_t* handle) {
  tcp_wrap_t* self = container_of(handle, tcp_wrap_t, handle);
  TSRMLS_D_GET(self);
  call_callback(self->close_cb, 0, NULL TSRMLS_CC);

  Z_DELREF_P(self->close_cb);

  if (self->connection_cb) {
    Z_DELREF_P(self->connection_cb);
  }
}


PHP_METHOD(TCP, close) {
  tcp_wrap_t* self;
  zval* callback;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &callback) == FAILURE) {
    return;
  }

  self = (tcp_wrap_t*) zend_object_store_get_object(getThis() TSRMLS_CC);
  HEALTHCHECK(self);

  self->close_cb = callback;
  Z_ADDREF_P(callback);

  uv_close((uv_handle_t*)&self->handle, tcp_close_cb);
  self->dead = 1;

  RETURN_NULL();
}


void tcp_connection_cb(uv_stream_t* server_handle, int status) {
  tcp_wrap_t* self = (tcp_wrap_t*) server_handle->data;
  tcp_wrap_t* client_wrap;
  zval* client_zval;
  int r;
  TSRMLS_D_GET(self);

  if (status != 0) {
    /* TODO: do something sensible */
    THROW_ERROR("Fuckup");
    return;
  }

  /* Create container for new object */
  MAKE_STD_ZVAL(client_zval);
  Z_TYPE_P(client_zval) = IS_OBJECT;
  Z_OBJVAL_P(client_zval) = tcp_new(tcp_ce TSRMLS_CC);
  client_wrap = (tcp_wrap_t*) zend_object_store_get_object(client_zval TSRMLS_CC);

  /* Accept connection */
  r = uv_accept(server_handle, (uv_stream_t*) &client_wrap->handle);
  if (r != 0) {
    /* This should not happen */
    THROW_ERROR("Mishap");
    return;
  }

  /* Call the connection callback */
  if (self->connection_cb) {
    zval* args[1];
    args[0] = client_zval;
    call_callback(self->connection_cb, 1, args TSRMLS_CC);
  }
};


PHP_METHOD(TCP, listen) {
  tcp_wrap_t* self;
  zval* arg1, *arg2, *arg3;
  zval* port, *host, *callback;
  struct sockaddr_in addr;
  int r;

  self = (tcp_wrap_t*) zend_object_store_get_object(getThis() TSRMLS_CC);
  HEALTHCHECK(self);

  if (self->listening) {
    THROW_ERROR("Already listening");
  }

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z!z!", &arg1, &arg2, &arg3) == FAILURE) {
    return;
  }

  if (Z_TYPE_P(arg1) == IS_LONG) {
    port = arg1;
  } else {
    /* No unix socket support yet */
    THROW_ERROR("Unix sockets are not supported yet");
    RETURN_NULL();
  }

  if (ZEND_NUM_ARGS() == 3) {
    host = arg2;
    callback = arg3;
  } else if (ZEND_NUM_ARGS() == 2) {
    if (Z_TYPE_P(arg2) == IS_STRING) {
      host = arg2;
      callback = NULL;
    } else {
      host = NULL;
      callback = arg2;
    }
  } else {
    callback = NULL;
    host = NULL;
  }

  if (host != NULL) {
    /* TODO: are php strings always null-terminated? */
    addr = uv_ip4_addr(Z_STRVAL_P(host), Z_LVAL_P(port));
  } else {
    addr = uv_ip4_addr("0.0.0.0", Z_LVAL_P(port));
  }

  r = uv_tcp_bind(&self->handle, addr);
  if (r != 0) {
    THROW_ERROR(uv_strerror(uv_last_error(self->handle.loop)));
    RETURN_NULL();
  }

  r = uv_listen((uv_stream_t*) &self->handle, 512, tcp_connection_cb);
  if (r != 0) {
    THROW_ERROR(uv_strerror(uv_last_error(self->handle.loop)));
    RETURN_NULL();
  }

  self->listening = 1;
  if (callback) {
    self->connection_cb = callback;
    Z_ADDREF_P(callback);
  }

  RETURN_NULL();
}


static zend_function_entry tcp_methods[] = {
  PHP_ME(TCP, connect, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(TCP, write, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(TCP, close, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(TCP, listen, NULL, ZEND_ACC_PUBLIC)
  { NULL }
};


PHP_MINIT_FUNCTION(phode) {
  zend_class_entry ce;

  uv_init();

  INIT_CLASS_ENTRY(ce, "TCP", tcp_methods);
  ce.create_object = tcp_new;
  tcp_ce = zend_register_internal_class(&ce TSRMLS_CC);

  return SUCCESS;
}


PHP_MSHUTDOWN_FUNCTION(phode) {
  return SUCCESS;
}


PHP_MINFO_FUNCTION(phode) {
  php_info_print_table_start();
  php_info_print_table_header(2, "phode", "enabled");
  php_info_print_table_end();
}


PHP_FUNCTION(uv_run) {
  uv_run(uv_default_loop());
  RETURN_NULL();
}


static zend_function_entry functions[] = {
  PHP_FE(uv_run, NULL)
  { NULL, NULL, NULL }
};


zend_module_entry phode_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
  STANDARD_MODULE_HEADER,
#endif
  "phode",
  functions,
  PHP_MINIT(phode),
  PHP_MSHUTDOWN(phode),
  NULL,
  NULL,
  PHP_MINFO(phode),
#if ZEND_MODULE_API_NO >= 20010901
  "0.0.1",
#endif
  STANDARD_MODULE_PROPERTIES
};


ZEND_GET_MODULE(phode)
