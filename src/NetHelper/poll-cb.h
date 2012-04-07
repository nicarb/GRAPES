#ifndef POLL_CB_H
#define POLL_CB_H

typedef struct pollcb * pollcb_t;

pollcb_t pollcb_new (void * (* event_cb) (void *), void * ctx,
                     void (* ctx_free_cb) (void *));

void pollcb_del (pollcb_t p);

void pollcb_run (pollcb_t p);

#endif // POLL_CB_H

