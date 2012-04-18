#include "poll-send.h"
#include "poll-cb.h"
#include "utils.h"
#include "nh-types.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/epoll.h>
#include <errno.h>
#include <string.h>

typedef enum {
    IDLE,
    SENDING_HDR,
    SENDING_MSG,
    ERROR
} state_t;

struct poll_send {
    state_t state;

    uint8_t * buffer;
    size_t buflen;
    header_t hdr;
    size_t sent;

    pollcb_t pcb;
};

static void * poll_callback (void *ctx, int fd, int epollfd);
static inline int can_send_more (int fd);

poll_send_t poll_send_new (int fd, int epollfd)
{
    poll_send_t ret;

    ret = mem_new(sizeof(struct poll_send));

    ret->state = IDLE;
    ret->buffer = NULL;
    ret->pcb = pollcb_new(poll_callback, (void *)ret, fd, epollfd);

    return ret;
}

int poll_send_is_alive (poll_send_t ps)
{
    return pollcb_is_alive(ps->pcb);
}

poll_send_res_t poll_send_enqueue (poll_send_t ps, const msg_buf_t *msg)
{
    switch (ps->state) {
        case IDLE:
            break;
        case ERROR:
            return POLL_SEND_FAIL;
        default:
            return POLL_SEND_BUSY;
    }

    ps->buffer = (uint8_t *) mem_renew((void *)ps->buffer, msg->size);
    memcpy((void *)ps->buffer, msg->data, msg->size);
    ps->buflen = msg->size;
    ps->sent = 0;
    header_set_size(&ps->hdr, msg->size);
    ps->state = SENDING_HDR;

    if (pollcb_enable(ps->pcb, EPOLLOUT) == -1) {
        return POLL_SEND_FAIL;
    }

    return POLL_SEND_SUCCESS;
}

void poll_send_del (poll_send_t ps)
{
    if (ps == NULL) return;
    pollcb_del(ps->pcb);
    free(ps->buffer);
    free(ps);
}

static
void * poll_callback (void *ctx, int fd, int _epollfd)
{
    poll_send_t ps;
    ssize_t n;
    int can_send;

    ps = (poll_send_t) ctx;
    can_send = 1;
    while (can_send) {
        switch (ps->state) {
            case IDLE:
                return ctx;
            case SENDING_HDR:
                n = write(fd, (uint8_t *)(&ps->hdr) + ps->sent,
                          sizeof(header_t) - ps->sent);
                if (n == -1) {
                    print_err("poll-send", "write", errno);
                    ps->state = ERROR;
                    return ctx;
                }
                ps->sent += n;
                if (ps->sent == sizeof(header_t)) {
                    ps->state = SENDING_MSG;
                    ps->sent = 0;
                }
                break;
            case SENDING_MSG:
                n = write(fd, ps->buffer + ps->sent, ps->buflen - ps->sent);
                if (n == -1) {
                    print_err("poll-send", "write", errno);
                    ps->state = ERROR;
                    return ctx;
                }
                ps->sent += n;
                if (ps->sent == ps->buflen) {
                    ps->state = IDLE;
                    if (pollcb_disable(ps->pcb) == -1) {
                        ps->state = ERROR;
                        return ctx;
                    }
                }
        }
        can_send = can_send_more(fd);
        if (can_send == -1) {
            ps->state = ERROR;
            return ctx;
        }
    }

    return ctx;
}

static inline
int can_send_more (int fd)
{
    fd_set W;
    struct timeval zero = {
        .tv_sec = 0,
        .tv_usec = 0
    };

    FD_ZERO(&W);
    FD_SET(fd, &W);
    return select(fd + 1, NULL, &W, NULL, &zero);
}
