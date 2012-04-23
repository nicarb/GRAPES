#include <stdlib.h>
#include <sys/epoll.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>

#include "utils.h"
#include "poll-cb.h"

struct pollcb {
    pollcb_cb_t cb;
    void * ctx;
    int epollfd;
    int fd;

    unsigned enabled : 1;
    unsigned dead : 1;
};

static void flag_dead (pollcb_t p);

pollcb_t pollcb_new (pollcb_cb_t cb, void * ctx, int fd, int epollfd)
{
    pollcb_t ret;

    if (invalid_fd(fd) || invalid_fd(epollfd)) {
        return NULL;
    }

    ret = mem_new(sizeof(struct pollcb));

    ret->fd = dup(fd);
    if (ret->fd == -1) {
        pollcb_del(ret);
        return NULL;
    }

    ret->cb = cb;
    ret->ctx = ctx;
    ret->epollfd = epollfd;
    ret->enabled = 0;
    ret->dead = 0;

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
    if (invalid_fd(p->fd) || invalid_fd(p->epollfd)) {
        flag_dead(p);
    } else if (!p->dead) {
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
        flag_dead(p);
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

    p->enabled = 0;
    if (epoll_ctl(p->epollfd, EPOLL_CTL_DEL, p->fd, &ev) == -1) {
        print_err("epoll_ctl", "del", errno);
        flag_dead(p);
        return -1;
    }

    return 0;
}

int pollcb_is_alive (pollcb_t p)
{
    if (invalid_fd(p->fd) || invalid_fd(p->epollfd)) {
        flag_dead(p);
    }
    return p->enabled || !p->dead;
}

static
void flag_dead (pollcb_t p)
{
    if (p->enabled) {
        pollcb_disable(p);
    }
    p->dead = 1;
}
