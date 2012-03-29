/*
 *  Copyright (c) 2012 Arber Fama,
 *                     Giovanni Simoni
 *
 *  This is free software; see lgpl-2.1.txt
 */

#include <dacav/dacav.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>

#include "dictionary.h"
#include "timeout.h"

struct peer_info {
    int fd;
    tout_t timeout;
    struct {
        unsigned used : 1;
    } flags;
};

/* -- Constants ------------------------------------------------------ */

static const char * CONF_KEY_NBUCKETS = "tcp_hashbuckets";
static const char * CONF_KEY_TIMEOUT = "tcp_socktimeout";

// Arbitrary and reasonable(?) prime number.
static const unsigned DEFAULT_N_BUCKETS = 23;
static const unsigned DEFAULT_TIMEOUT_MINUTES = 10;

/* -- Internal functions --------------------------------------------- */

static inline int invalid_fd (int fd);

/* -- Interface exported symbols ------------------------------------- */

struct dict {
    dhash_t *ht;
    size_t count;
    unsigned tout_minutes;
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
    return create_copy(s, sizeof(struct peer_info));
}

static
void peer_info_free (void *s)
{
    peer_info_t info = s;

    tout_del(info->timeout);
    close(info->fd);
    free(s);
}

dict_t dict_new (int af, int autoclose, struct tag *cfg_tags)
{
    int nbks;
    unsigned tout_minutes;
    dict_t ret;
    dhash_cprm_t addr_cprm, peer_info_cprm;

    if (af != AF_INET) {
        fprintf(stderr, "dictionary: Only AF_INET is supported for the"
                        " moment\n");
        abort();
    }

    nbks = DEFAULT_N_BUCKETS;
    tout_minutes = DEFAULT_TIMEOUT_MINUTES;
    if (cfg_tags != NULL) {
        config_value_int_default(cfg_tags, CONF_KEY_NBUCKETS, &nbks,
                                 DEFAULT_N_BUCKETS);
        config_value_int_default(cfg_tags, CONF_KEY_TIMEOUT,
                                 &tout_minutes, DEFAULT_TIMEOUT_MINUTES);
    }

    ret = malloc(sizeof(struct dict));
    assert(ret != NULL);

    addr_cprm.cp = sockaddr_copy;
    addr_cprm.rm = free;
    peer_info_cprm.cp = peer_info_copy;
    peer_info_cprm.rm = autoclose ? peer_info_free : free;

    ret->ht = dhash_new(nbks, sockaddr_hash, sockaddr_cmp,
                        &addr_cprm, &peer_info_cprm);
    ret->count = 0;
    ret->tout_minutes = tout_minutes;

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

int dict_lookup (dict_t D, const struct sockaddr *addr,
                 peer_info_t *info)
{
    if (dhash_search(D->ht, (const void *)addr, (void **)info)
            == DHASH_FOUND) {
        return invalid_fd((*info)->fd) ? -1 : 0;
    } else {
        return -1;
    }
}

struct lookup_data {
    int (* make_socket) (void *ctx);
    void *ctx;
};

static
void * build_peer_info (void * param)
{
    struct lookup_data *ld = param;
    peer_info_t info = malloc(sizeof(struct peer_info));

    assert(info != NULL);
    info->flags.used = 0;
    info->fd = ld->make_socket(ld->ctx);

    return (void *)info;
}

void dict_lookup_default (dict_t D, const struct sockaddr *addr,
                          peer_info_t info,
                          int (* make_socket) (void *ctx), void *ctx)
{
    /* NOTE: currently this code is NOT used. If enabling this feature,
     * please double-check the code correctness:
     *
     * - is file descriptor valid?
     * - compare with dict_lookup
     */
    struct lookup_data ld = {
        .make_socket = make_socket,
        .ctx = ctx
    };
    dhash_search_default(D->ht, (const void *)addr, (void **)&info,
                         build_peer_info, (void *) &ld);
}

int dict_insert (dict_t D, const struct sockaddr * addr, int fd)
{
    struct peer_info info;
    struct timeval tout;

    info.fd = fd;
    info.flags.used = 0;

    tout.tv_usec = 0;
    tout.tv_sec = 60 * D->tout_minutes;
    info.timeout = tout_new(&tout);

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
        dhash_pair_t *P;
        peer_info_t info;
        int garbage;

        P = diter_next(it);
        info = dhash_val(P);

        if (invalid_fd(info->fd) || tout_expired(info->timeout)) {
            diter_remove(it, NULL);
            D->count --;
        } else {
            switch (cback(ctx, dhash_key(P), info)) {
                case DICT_SCAN_DEL_CONTINUE:
                    D->count --;
                    diter_remove(it, NULL);
                case DICT_SCAN_CONTINUE:
                    break;
                case DICT_SCAN_DEL_STOP:
                    D->count --;
                    diter_remove(it, NULL);
                case DICT_SCAN_STOP:
                    go = 0;
                    break;
            }
        }
    }
    dhash_iter_free(it);
}

int dict_get_fd (const peer_info_t info)
{
    return info->fd;
}

int dict_is_used (const peer_info_t info)
{
    return info->flags.used;
}

int dict_reset_used (peer_info_t info)
{
    int how_was = info->flags.used;
    info->flags.used = 0;
    return how_was;
}

int dict_set_used (peer_info_t info)
{
    int how_was = info->flags.used;
    info->flags.used = 1;
    return how_was;
}

void dict_refresh (peer_info_t info)
{
    tout_reset(info->timeout);
}

/* -- Internal functions --------------------------------------------- */

static inline
int invalid_fd (int fd)
{
    return fcntl(fd, F_GETFL) == -1;
}
