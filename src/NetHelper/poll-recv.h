#ifndef POLL_RECV_H
#define POLL_RECV_H

#include <stddef.h>

typedef struct poll_recv * poll_recv_t;

typedef struct {
    const void * msg;
    size_t size;
} poll_recv_buf_t;

typedef enum {
    POLL_RECV_FAIL = -1,
    POLL_RECV_SUCCESS = 0,
    POLL_RECV_BUSY = 1
} poll_recv_res_t;

poll_recv_t poll_recv_new (int fd, int epollfd);

poll_recv_res_t poll_recv_retrieve (poll_recv_t pr,
                                    const poll_recv_buf_t **out);

void poll_recv_del (poll_recv_t ps);

#endif // POLL_RECV_H

