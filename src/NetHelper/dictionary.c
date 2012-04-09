/*
 *  Copyright (c) 2012 Arber Fama,
 *                     Giovanni Simoni
 *
 *  This is free software; see lgpl-2.1.txt
 */

#include <dacav/dacav.h>

#include <stdlib.h>
#include <unistd.h>

#include "utils.h"
#include "dictionary.h"
#include "timeout.h"
#include "sockaddr-helpers.h"

struct dict {
    dhash_t *hash;
    size_t count;
    unsigned tout_minutes;
};

struct dict_data {
    tout_t timeout;
    int fd;
    struct {
        void *bytes;
        size_t size;
    } buffer;
};

static const unsigned DEFAULT_BUCKETS = 17;
static const unsigned DEFAULT_TOUT_MINUTES = 10;

static const char CONF_KEY_BUCKETS[] = "TCPHashBuckets";
static const char CONF_KEY_TOUT_MINUTES[] = "TCPTimeoutMinutes";

static void * dict_data_new (void * ctx);
static void dict_data_del (void * ptr);
static void * dict_data_copy (const void * ptr);
static void reset_if_timeout (dict_data_t dd);

dict_t dict_new (struct tag *cfg)
{
    dict_t d;
    int nbuckets, tout_minutes;
    dcprm_t cprm_k;
    dcprm_t cprm_v;

    nbuckets = DEFAULT_BUCKETS;
    tout_minutes = DEFAULT_TOUT_MINUTES;
    if (cfg) {
        config_value_int_default(cfg, CONF_KEY_BUCKETS, &nbuckets,
                                 DEFAULT_BUCKETS);
        config_value_int_default(cfg, CONF_KEY_TOUT_MINUTES,
                                 &tout_minutes, DEFAULT_TOUT_MINUTES);
    }

    cprm_k.cp = (dcopy_cb_t) sockaddr_copy;
    cprm_k.rm = free;

    cprm_v.cp = dict_data_copy;
    cprm_v.rm = dict_data_del;

    d = mem_new(sizeof(struct dict));
    d->hash = dhash_new(nbuckets,
                        (dhash_cb_t) sockaddr_hash,
                        (dcmp_cb_t) sockaddr_cmp,
                        &cprm_k, &cprm_v);
    d->count = 0;
    d->tout_minutes = tout_minutes;

    return d;
}

void dict_del (dict_t d)
{
    dhash_free(d->hash);
}

dict_data_t dict_search (dict_t d, const struct sockaddr *addr)
{
    dict_data_t out;

    dhash_search_default(d->hash, (const void *)addr, (void **) &out,
                         dict_data_new, (void *)d);
    reset_if_timeout(out);
    return out;
}

int * dict_data_fd (dict_data_t dd)
{
    return &dd->fd;
}

uint8_t * dict_data_buffer (dict_data_t dd, size_t size)
{
    if (dd->buffer.size < size) {
        dd->buffer.bytes = mem_renew(dd->buffer.bytes, size);
        dd->buffer.size = size;
    }

    return (uint8_t *) dd->buffer.bytes;
}

void dict_data_update (dict_data_t dd)
{
    tout_reset(dd->timeout);
}

static void * dict_data_new (void * ctx)
{
    dict_t dict;
    dict_data_t ret;
    struct timeval tout;

    dict = (dict_t) ctx;
    tout.tv_sec = dict->tout_minutes * 60;
    tout.tv_usec = 0;

    ret = mem_new(sizeof(struct dict_data));
    ret->timeout = tout_new(&tout);
    ret->fd = -1;
    ret->buffer.size = 0;
    ret->buffer.bytes = NULL;

    return (void *)ret;
}

static void * dict_data_copy (const void * ptr)
{
    /* If doing this there's something wrong... */
    abort();
}

static void dict_data_del (void * ptr)
{
    dict_data_t d;

    d = (dict_data_t) ptr;
    tout_del(d->timeout);
    free(d->buffer.bytes);
}

static void reset_if_timeout (dict_data_t dd)
{
    /* Makes dd as it were just created */
    if (tout_expired(dd->timeout)) {
        close(dd->fd);
        dd->fd = -1;
        tout_reset(dd->timeout);
    }
}
