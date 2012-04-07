#include "poll-cb.h"
#include <stdlib.h>

struct pollcb {
    void * (*event_cb) (void *);
    void *ctx;
    void (*free_cb) (void *);
};

pollcb_t pollcb_new (void * (* event_cb) (void *), void * ctx,
                     void (* free_cb) (void *))
{
    pollcb_t ret;

    ret = malloc(sizeof(struct pollcb));
    assert(ret != NULL);

    ret->event_cb = event_cb;
    ret->ctx = ctx;
    ret->free_cb = free_cb;

    return ret;
}

void pollcb_del (pollcb_t p)
{
    if (p->free_cb) {
        p->free_cb(p->ctx);
    }
    free(p);
}

void pollcb_run (pollcb_t p)
{
    p->ctx = p->event_cb(p->ctx);
}
