#include <stdlib.h>
#include <sys/epoll.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

#include "utils.h"
#include "poll-cb.h"

struct pollcb {
    pollcb_cb_t cb;
    void * ctx;
    int epollfd;
    int fd;

    unsigned enabled : 1;
    unsigned seppuku : 1;
};

pollcb_t pollcb_new (pollcb_cb_t cb, void * ctx, int fd, int epollfd)
{
    pollcb_t ret;

    ret = mem_new(sizeof(struct pollcb));
    ret->cb = cb;
    ret->ctx = ctx;
    ret->epollfd = epollfd;
    ret->enabled = 0;
    ret->seppuku = 0;
    ret->fd = dup(fd);

    if (ret->fd == -1) {
        pollcb_del(ret);
        return NULL;
    }

    return ret;
}

void pollcb_del (pollcb_t p)
{
    if (p == NULL) return;

    if (p->enabled) {
        p->seppuku = 1;
    } else {
        close(p->fd);
        free(p);
    }
}

void pollcb_run (pollcb_t p)
{
    if (p->seppuku) {
        assert(p->enabled);
        pollcb_disable(p);
        p->seppuku = 0;
        pollcb_del(p);
    } else {
        p->ctx = p->cb(p->ctx, p->fd, p->epollfd);
    }
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
    p->enabled = 1;
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
    p->enabled = 0;

    return 0;
}

int pollcb_is_alive (pollcb_t p)
{
    if (p->enabled) return 1;

    /* Both file descriptors are still valid */
    return (fcntl(p->fd, F_GETFD) != -1) &&
           (fcntl(p->epollfd, F_GETFD) != -1);
}

