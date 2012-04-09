#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <stdint.h>

/* like calling malloc, just abort()s if return null */
void * mem_new (size_t size);

/* like calling realloc, just abort()s if return null */
void * mem_renew (void * ptr, size_t size);

/* allocate a copy of the given memory area */
void * mem_dup (const void * src, size_t size);

/* print error on standard error */
void print_err (const char *where, const char *what, int e);

/* send until completed, return 0 or -1 on success and failure
 * respectively */
int send_forced (int fd, const uint8_t * buffer, size_t buflen);

/* receive until completed, return 0 or -1 on success and failure
 * respectively */
int recv_forced (int fd, uint8_t * buffer, size_t buflen);

#endif // UTILS_H

