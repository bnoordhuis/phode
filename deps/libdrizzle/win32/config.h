#ifndef __DRIZZLE_WIN32_CONFIG_H
#define __DRIZZLE_WIN32_CONFIG_H


#define HAVE_FCNTL_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRING_H

typedef unsigned int in_port_t;
typedef enum { false = 0, true = 1 } _Bool;
typedef _Bool bool;

struct sockaddr_un
{
  short int sun_family;
  char sun_path[108];
};

#define snprintf _snprintf

#define ECONNREFUSED WSAECONNRESET

#endif /* __DRIZZLE_WIN32_CONFIG_H */
