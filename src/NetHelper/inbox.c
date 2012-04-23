#include "inbox.h"

#include <dacav/dacav.h>
#include <stdlib.h>
#include <assert.h>

#include "utils.h"
#include "sockaddr-helpers.h"
#include "dictionary.h"

struct inbox {
    dlist_t * queue;
};

static int scan_callback (void *ctx, const sockaddr_t *addr, client_t cl);

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
    if (ib == NULL) return;
    /* Note: client_t items are already stored in the dictionary. They are
     *       just kept in a queue here. And yes, this is safe */
    dlist_free(ib->queue, NULL);
    free(ib);
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
int scan_callback (void *ctx, const sockaddr_t *addr, client_t cl)
{
    inbox_t ib = (inbox_t) ctx;

    assert(sockaddr_equal(addr, client_get_addr(cl)));

    if (!client_has_message(cl)) return 1;
    if (client_flag_enqueued(cl, 1)) return 1;
    ib->queue = dlist_append(ib->queue, (void *)cl);
    return 1;   /* Don't stop */
}

