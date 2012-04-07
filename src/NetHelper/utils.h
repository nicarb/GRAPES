#ifndef UTILS_H
#define UTILS_H

void * mem_new (size_t size);

void * mem_dup (const void * src, size_t size);

void print_err (const char *where, const char *what, int e);

#endif // UTILS_H

