#include "utils.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

void * mem_renew (void * ptr, size_t size)
{
    void * ret = realloc(ptr, size);
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

int send_forced (int fd, const uint8_t * buffer, size_t buflen)
{
    while (buflen > 0) {
        ssize_t n = write(fd, (const void *)buffer, buflen);
        if (n == -1) {
            print_err("Send forced", "send", errno);
            return -1;
        }
        buflen -= n;
        buffer += n;
    }
    return 0;
}

int recv_forced (int fd, uint8_t * buffer, size_t buflen)
{
    while (buflen > 0) {
        ssize_t n = read(fd, (void *)buffer, buflen);
        if (n == -1) {
            print_err("Receive forced", "recv", errno);
            return -1;
        }
        buflen -= n;
        buffer += n;
    }

    return 0;
}

