#ifndef SOCKADDR_HELPERS_H
#define SOCKADDR_HELPERS_H

#include <netinet/in.h>
#include <stdint.h>

size_t sockaddr_size (const struct sockaddr *);

uintptr_t sockaddr_hash (const struct sockaddr *);

int sockaddr_cmp (const struct sockaddr *sa0,
                  const struct sockaddr *sa1);

int sockaddr_equal (const struct sockaddr *sa0,
                    const struct sockaddr *sa1);

int sockaddr_dump (void *dst, size_t dstsize, const struct sockaddr *src);

int sockaddr_undump (struct sockaddr *dst, size_t dstsize,
                     const void *src);

struct sockaddr * sockaddr_copy (struct sockaddr *dst, 
                                 const struct sockaddr * src);

struct sockaddr * sockaddr_dup (const struct sockaddr *src);

int sockaddr_strrep (const struct sockaddr *, char *buffer, size_t buflen);

int sockaddr_send_hello (const struct sockaddr *ouraddr, int fd);

int sockaddr_recv_hello (struct sockaddr *theiraddr, int fd);

/* Specific calls for sockaddr_in */

int sockaddr_in_init (struct sockaddr_in *in, const char *ipaddr,
                      uint16_t port);

#endif // SOCKADDR_HELPERS_H

