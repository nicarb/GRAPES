#ifndef POLL_CB_H
#define POLL_CB_H

typedef struct pollcb * pollcb_t;

pollcb_t pollcb_new (void * (* event_cb) (void *, int), void * ctx);

void pollcb_del (pollcb_t p);

void pollcb_run (pollcb_t p, int epollfd);

#endif // POLL_CB_H

