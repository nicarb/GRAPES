#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

/* like calling malloc, just abort()s if return null */
void * mem_new (size_t size);

/* like calling realloc, just abort()s if return null */
void * mem_renew (void * ptr, size_t size);

/* allocate a copy of the given memory area */
void * mem_dup (const void * src, size_t size);

/* print error on standard error */
void print_err (const char *where, const char *what, int e);

#endif // UTILS_H

