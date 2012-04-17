#include <stdlib.h>
#include <sys/epoll.h>
#include <errno.h>
#include <unistd.h>

#include "utils.h"
#include "poll-cb.h"

struct pollcb {
    pollcb_cb_t cb;
    void * ctx;
    int epollfd;
    int fd;
};

pollcb_t pollcb_new (pollcb_cb_t cb, void * ctx, int fd, int epollfd)
{
    pollcb_t ret;

    ret = mem_new(sizeof(struct pollcb));
    ret->cb = cb;
    ret->ctx = ctx;
    ret->epollfd = epollfd;
    ret->fd = dup(fd);

    return ret;
}

void pollcb_del (pollcb_t p)
{
    if (p == NULL) return;
    close(p->fd);
    free(p);
}

void pollcb_run (pollcb_t p)
{
    p->ctx = p->cb(p->ctx, p->fd, p->epollfd);
}

int pollcb_enable (pollcb_t p, uint32_t events)
{
    struct epoll_event ev = {
        .events = events,
        .data.ptr = (void *)p
    };

    if (epoll_ctl(p->epollfd, EPOLL_CTL_ADD, p->fd, &ev) == -1) {
        print_err("epoll_ctl", "add", errno);
        return -1;
    }
    return 0;
}

int pollcb_disable (pollcb_t p)
{
    struct epoll_event ev = {
        .events = 0,
        .data.ptr = NULL
    };

    if (epoll_ctl(p->epollfd, EPOLL_CTL_DEL, p->fd, &ev) == -1) {
        print_err("epoll_ctl", "del", errno);
        return -1;
    }
    return 0;
}

