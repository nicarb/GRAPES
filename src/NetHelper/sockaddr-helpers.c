#include "sockaddr-helpers.h"
#include "utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static
ssize_t get_size (const struct sockaddr *s)
{
    switch (s->sa_family) {
        case AF_INET:
            return sizeof(struct sockaddr_in);
        case AF_INET6:
            return sizeof(struct sockaddr_in6);
        default:
            return -1;
    }
}

size_t sockaddr_size (const struct sockaddr *s)
{
    size_t ret = get_size(s);
    if (ret == -1) {
        print_err("Address analysis", "family not supported", 0);
        abort();
    }
    return ret;
}

uintptr_t sockaddr_hash (const struct sockaddr *k)
{
    uintptr_t h, g;
    size_t nbytes;
    int i;

    nbytes = sockaddr_size(k);
    h = 0;
    for (i = 0; i < nbytes; i ++) {
        h = (h << 4) + ((const uint8_t *)k)[i];
        if ((g = h & 0xf00000000) != 0) {
            h &= g >> 24;
        }
        h &= ~g;
    }
    return h;
}

int sockaddr_cmp (const struct sockaddr *sa0, const struct sockaddr *sa1)
{
    size_t size0, size1;

    size0 = sockaddr_size(sa0);
    size1 = sockaddr_size(sa1);

    return memcmp(sa0, sa1, size0 < size1 ? size0 : size1);
}

int sockaddr_equal (const struct sockaddr *sa0,
                    const struct sockaddr *sa1)
{
    return sockaddr_cmp(sa0, sa1) == 0;
}

int sockaddr_dump (void *dst, size_t dstsize, const struct sockaddr *src)
{
    size_t len;

    len = get_size(src);
    if (dstsize < len) return -1;
    memcpy(dst, (const void *)src, len);

    return len;
}

int sockaddr_undump (struct sockaddr *dst, size_t dstsize,
                     const void *src)
{
    size_t len;

    len = get_size((const struct sockaddr *)src);
    if (dstsize < len) return -1;
    memcpy((void *)dst, src, len);

    return len;
}

struct sockaddr * sockaddr_copy (const struct sockaddr * src)
{
    return (struct sockaddr *) mem_dup((const void *)src,
                                       sockaddr_size(src));
}

int sockaddr_in_init (struct sockaddr_in *in, const char *ipaddr,
                      uint16_t port)
{
    memset(in, 0, sizeof(struct sockaddr_in));
    in->sin_family = AF_INET;
    in->sin_port = htons(port);

    /* Initialization of string representation */
    if (ipaddr == NULL) {
        /* In case of server, specifying NULL will allow anyone to
         * connect. */
        in->sin_addr.s_addr = INADDR_ANY;
    } else {
        if (inet_pton(AF_INET, ipaddr, (void *)&in->sin_addr) == 0) {
            print_err("Initializing sockaddr", "invalid address", 0);
            return -1;
        }
    }

    return 0;
}
