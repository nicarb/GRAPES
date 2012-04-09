#include "utils.h"

#include <stdlib.h>
#include <string.h>

void * mem_renew (void * ptr, size_t size)
{
    void * ret = reallo(ptr, size);
    if (ret == NULL) abort();
    return ret;
}

void * mem_new (size_t size)
{
    return mem_renew(NULL, size);
}

void * mem_dup (const void * src, size_t size)
{
    void * ret = mem_new(size);
    memcpy(ret, src, size);
    return ret;
}

void print_err (const char *where, const char *what, int e)
{
    if (e) {
        char buf[30];

        strerror_r(e, buf, 30);
        if (what) {
            fprintf(stderr, "%s: %s: %s\n", where, what, buf);
        } else {
            fprintf(stderr, "%s: %s\n", where, buf);
        }
    } else {
        fprintf(stderr, "%s: %s\n", where, what);
    }
}

