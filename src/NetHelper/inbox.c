#include "inbox.h"

#include <dacav/dacav.h>
#include <stdlib.h>

#include "utils.h"
#include "dictionary.h"

struct inbox {
    dlist_t * queue;
};

static int scan_callback (void *ctx, const struct sockaddr *addr,
                          void *userdata, uint32_t *flags);

inbox_t inbox_new ()
{
    inbox_t ib;

    ib = mem_new(sizeof(struct inbox));
    ib->queue = dlist_new();

    return ib;
}

int inbox_empty (inbox_t ib)
{
    return dlist_empty(ib->queue);
}

void inbox_del (inbox_t ib)
{
    /* Note: client_t items are already stored in the dictionary. They are
     *       just kept in a queue here. And yes, this is safe */
    dlist_free(ib->queue, NULL);
}

void inbox_scan_dict (inbox_t ib, dict_t dict)
{
    dict_foreach(dict, scan_callback, (void *)ib);
}

client_t inbox_next (inbox_t ib)
{
    client_t ret;

    ib->queue = dlist_pop(ib->queue, (void **) &ret);
    client_flag_enqueued(ret, 0);

    return ret;
}

static
int scan_callback (void *ctx, const struct sockaddr *addr, void *userdata,
                   uint32_t *flags)
{
    inbox_t ib;
    client_t cl;

    ib = (inbox_t) ctx;
    cl = (client_t) userdata;
    if (!client_flag_enqueued(cl, 1)) {
        /* Not in queue already */
        ib->queue = dlist_append(ib->queue, (void *)cl);
    }

    return 1;   /* Don't stop */
}

