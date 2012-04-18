#ifndef POLL_CB_H
#define POLL_CB_H

#include <stdint.h>

typedef struct pollcb * pollcb_t;

typedef void * (* pollcb_cb_t) (void *ctx, int fd, int epollfd);

/* NOTE: fd gets duplicated */
pollcb_t pollcb_new (pollcb_cb_t cb, void * ctx, int fd, int epollfd);

void pollcb_del (pollcb_t p);

void pollcb_run (pollcb_t p);

int pollcb_enable (pollcb_t p, uint32_t events);

int pollcb_disable (pollcb_t p);

int pollcb_is_alive (pollcb_t p);

#endif // POLL_CB_H

