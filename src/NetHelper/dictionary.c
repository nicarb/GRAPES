/*
 * Copyright (c) 2012 Giovanni Simoni, Arber Fama
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <dacav/dacav.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#include "dictionary.h"

/* -- Constants ------------------------------------------------------ */

static const char * CONF_KEY_NBUCKETS = "tcp_hashbuckets";

// Arbitrary and reasonable(?) prime number.
static const unsigned DEFAULT_N_BUCKETS = 23;

/* -- Interface exported symbols ------------------------------------- */

struct dict {
    dhash_t *ht;
    size_t count;
};

static
uintptr_t sockaddr_hash (const void *key)
{
    const struct sockaddr_in *K = key;
    return K->sin_addr.s_addr + K->sin_port;
}

static
int sockaddr_cmp (const void *v0, const void *v1)
{
    return memcmp(v0, v1, sizeof(struct sockaddr_in));
}

static
void * create_copy (const void *src, size_t size)
{
    void * dst = malloc(size);
    assert(dst != NULL);
    memcpy(dst, src, size);
    return dst;
}

static
void * sockaddr_copy (const void *s)
{
    return create_copy(s, sizeof(struct sockaddr_in));
}

static
void * peer_info_copy (const void *s)
{
    return create_copy(s, sizeof(peer_info_t));
}

static
void peer_info_free (void *s)
{
    close(((peer_info_t *)s)->fd);
    free(s);
}

dict_t dict_new (int af, int autoclose, struct tag *cfg_tags)
{
    if (af != AF_INET) {
        fprintf(stderr, "dictionary: Only AF_INET is supported for the"
                        " moment\n");
        abort();
    }

    int nbks = DEFAULT_N_BUCKETS;
    if (cfg_tags != NULL) {
        config_value_int_default(cfg_tags, CONF_KEY_NBUCKETS, &nbks,
                                 DEFAULT_N_BUCKETS);
    }

    dict_t ret = malloc(sizeof(struct dict));
    assert(ret != NULL);

    dhash_cprm_t addr_cprm = {
        .cp = sockaddr_copy,
        .rm = free
    };
    dhash_cprm_t peer_info_cprm = {
        .cp = peer_info_copy,
        .rm = autoclose ? peer_info_free : free
    };
    ret->ht = dhash_new(nbks, sockaddr_hash, sockaddr_cmp,
                        &addr_cprm, &peer_info_cprm);
    return ret;
}

void dict_delete (dict_t D)
{
    dhash_free(D->ht);
    free(D);
}

size_t dict_size (dict_t D)
{
    return D->count;
}

int dict_lookup (dict_t D, const struct sockaddr * addr,
                 peer_info_t *info)
{
    return dhash_search(D->ht, (const void *)addr, (void *)&info)
           == DHASH_FOUND ? 0 : -1;
}

int dict_insert (dict_t D, const struct sockaddr * addr, int fd)
{
    peer_info_t info;
    info.fd = fd;
    info.flags.used = 0;

    if (dhash_insert(D->ht, (const void *)addr, (void *)&info)
            == DHASH_FOUND) {
        return 1;
    } else {
        D->count ++;
        return 0;
    }
}

int dict_remove (dict_t D, const struct sockaddr * addr)
{
    if (dhash_delete(D->ht, (const void *)addr, NULL)
            == DHASH_FOUND) {
        D->count --;
        return 0;
    } else {
        return -1;
    }
}

void dict_scan (dict_t D, dict_scancb_t cback, void *ctx)
{
    diter_t *it = dhash_iter_new(D->ht);
    int go = 1;
    while (go && diter_hasnext(it)) {
        dhash_pair_t *P = diter_next(it);

        switch (cback(ctx, dhash_key(P), dhash_val(P))) {
            case DICT_SCAN_DEL_CONTINUE:
                diter_remove(it, NULL);
            case DICT_SCAN_CONTINUE:
                break;
            case DICT_SCAN_DEL_STOP:
                diter_remove(it, NULL);
            case DICT_SCAN_STOP:
                go = 0;
                break;
        }
    }
    dhash_iter_free(it);
}

