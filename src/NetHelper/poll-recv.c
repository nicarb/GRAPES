#include "poll-recv.h"
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
    RECEIVING_HDR,
    RECEIVING_MSG,
    COMPLETE,
    STOP,
    ERROR
} state_t;

struct poll_recv {
    state_t state;

    struct {
        uint8_t * bytes;
        size_t size;
    } buffer;

    header_t hdr;
    size_t received;

    pollcb_t pcb;
};

static void * poll_callback (void *ctx, int fd, int epollfd);
static inline int can_recv_more (int fd);
static void ensure_buffer_size (poll_recv_t pr, size_t size);

poll_recv_t poll_recv_new (int fd, int epollfd)
{
    poll_recv_t ret;

    ret = mem_new(sizeof(struct poll_recv));

    ret->state = RECEIVING_HDR;
    ret->buffer.bytes = NULL;
    ret->buffer.size = 0;
    ret->received = 0;
    ret->pcb = pollcb_new(poll_callback, (void *)ret, fd, epollfd);

    if (pollcb_enable(ret->pcb, EPOLLIN) == -1) {
        poll_recv_del(ret);
        return NULL;
    }

    return ret;
}

int poll_recv_has_message (poll_recv_t pr)
{
    return pr->state == COMPLETE;
}

int poll_recv_is_alive (poll_recv_t pr)
{
    switch (pr->state) {
        case COMPLETE:
            return 1;
        case ERROR:
        case STOP:
            return 0;
        default:
            return pollcb_is_alive(pr->pcb);
    }
}

poll_recv_res_t poll_recv_retrieve (poll_recv_t pr,
                                    const msg_buf_t **out)
{
    *out = NULL;
    switch (pr->state) {
        case COMPLETE:
            break;
        case ERROR:
            return POLL_RECV_FAIL;
        case STOP:
            return POLL_RECV_STOP;
        default:
            return POLL_RECV_BUSY;
    }

    if (pollcb_enable(pr->pcb, EPOLLIN) == -1) {
        pr->state = ERROR;
    }

    pr->state = RECEIVING_HDR;
    *out = (const msg_buf_t *) &pr->buffer;
    return POLL_RECV_SUCCESS;
}

void poll_recv_del (poll_recv_t pr)
{
    if (pr == NULL) return;

    pollcb_del(pr->pcb);
    free(pr->buffer.bytes);
    free(pr);
}

static
void * poll_callback (void *ctx, int fd, int _epollfd)
{
    poll_recv_t pr;
    ssize_t n;
    int have_data;

    pr = (poll_recv_t) ctx;
    have_data = 1;
    while (have_data) {
        switch (pr->state) {
            case COMPLETE:
            case ERROR:
                return ctx;
            case RECEIVING_HDR:
                n = read(fd, (uint8_t *)(&pr->hdr) + pr->received,
                         sizeof(header_t) - pr->received);
                if (n == -1) {
                    print_err("poll-recv", "read", errno);
                    pr->state = ERROR;
                    return ctx;
                }
                pr->received += n;
                if (pr->received == sizeof(header_t)) {
                    ssize_t size = header_get_size(&pr->hdr);

                    if (size == -1) {
                        pr->state = ERROR;
                        return ctx;
                    }
                    ensure_buffer_size(pr, size);
                    pr->state = RECEIVING_MSG;
                    pr->received = 0;
                }
                break;
            case RECEIVING_MSG:
                n = read(fd, pr->buffer.bytes + pr->received,
                         pr->buffer.size - pr->received);
                if (n == -1) {
                    print_err("poll-recv", "read", errno);
                    pr->state = ERROR;
                    return ctx;
                }
                pr->received += n;
                if (pr->received == pr->buffer.size) {
                    pr->state = COMPLETE;
                    pr->received = 0;
                    if (pollcb_disable(pr->pcb) == -1) {
                        pr->state = ERROR;
                    }
                    return ctx;
                }
        }
        if (n == 0) {
            pr->state = STOP;
            return ctx;
        }
        have_data = can_recv_more(fd);
        if (have_data == -1) {
            pr->state = ERROR;
            return ctx;
        }
    }

    return ctx;
}

static inline
int can_recv_more (int fd)
{
    fd_set R;
    struct timeval zero = {
        .tv_sec = 0,
        .tv_usec = 0
    };

    FD_ZERO(&R);
    FD_SET(fd, &R);
    return select(fd + 1, &R, NULL, NULL, &zero);
}

static
void ensure_buffer_size (poll_recv_t pr, size_t size)
{
    /* Here insert a reasonable buffering policy of yours :) */
    pr->buffer.bytes = (uint8_t *) mem_renew((void *) pr->buffer.bytes,
                                             size);
    pr->buffer.size = size;
}

