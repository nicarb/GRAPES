#include <stdlib.h>

#include "utils.h"
#include "poll-cb.h"

struct pollcb {
    void * (*event_cb) (void *, int);
    void * ctx;
};

pollcb_t pollcb_new (void * (* event_cb) (void *, int), void * ctx)
{
    pollcb_t ret;

    ret = mem_new(sizeof(struct pollcb));
    ret->event_cb = event_cb;
    ret->ctx = ctx;

    return ret;
}

void pollcb_del (pollcb_t p)
{
    free(p);
}

void pollcb_run (pollcb_t p, int epollfd)
{
    p->ctx = p->event_cb(p->ctx, epollfd);
}
