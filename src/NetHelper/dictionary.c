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
    struct {
        dict_delcb_t del;
        dict_pred_t valid;
    } cbacks;
};

struct dict_data {
    tout_t timeout;
    uint32_t flags;
    struct {
        void *data;
        dict_delcb_t del;
    } user;
};

static const unsigned DEFAULT_BUCKETS = 17;
static const unsigned DEFAULT_TOUT_MINUTES = 10;

static const char CONF_KEY_BUCKETS[] = "TCPHashBuckets";
static const char CONF_KEY_TOUT_MINUTES[] = "TCPTimeoutMinutes";

static void * dict_data_new (void * ctx);
static void dict_data_del (void * ptr);
static void * dict_data_copy (const void * ptr);
static int dict_data_valid (dict_data_t data, dict_pred_t user_check);
static void dict_data_reset (dict_data_t data);

dict_t dict_new (struct tag *cfg, dict_delcb_t del, dict_pred_t valid)
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
    d->cbacks.del = del;
    d->cbacks.valid = valid;

    return d;
}

void dict_del (dict_t d)
{
    if (d == NULL) return;
    dhash_free(d->hash);
}

dict_data_t dict_search (dict_t d, const struct sockaddr *addr)
{
    dict_data_t out;

    dhash_search_default(d->hash, (const void *)addr, (void **) &out,
                         dict_data_new, (void *)d);
    if (!dict_data_valid(out, d->cbacks.valid)) {
        dict_data_reset(out);
    }
    return out;
}

void dict_data_update (dict_data_t dd)
{
    tout_reset(dd->timeout);
}

void ** dict_data_user (dict_data_t dd)
{
    return &dd->user.data;
}

void dict_foreach (dict_t d, dict_foreach_t cb, void * ctx)
{
    diter_t *iter;

    iter = dhash_iter_new(d->hash);
    while (diter_hasnext(iter)) {
        dhash_pair_t *P;
        dict_data_t data;

        P = diter_next(iter);
        data = dhash_val(P);
        if (!dict_data_valid(data, d->cbacks.valid)) {
            diter_remove(iter);
            continue;
        }
        if (cb(ctx, dhash_key(P), data->user.data, &data->flags) == 0) {
            break;
        }
    }
    dhash_iter_free(iter);
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
    ret->flags = 0;
    ret->user.data = NULL;
    ret->user.del = dict->cbacks.del;

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

    if (ptr == NULL) return;
    d = (dict_data_t) ptr;
    tout_del(d->timeout);
    if (d->user.del) {
        d->user.del(d->user.data);
    }
}

static int dict_data_valid (dict_data_t data, dict_pred_t user_check)
{
    if (tout_expired(data->timeout)) return 0;
    if (user_check && !user_check(data->user.data)) return 0;
    return 1;
}

static void dict_data_reset (dict_data_t data)
{
    tout_reset(data->timeout);
    if (data->user.del) {
        data->user.del(data->user.data);
    }
    data->user.data = NULL;
}

