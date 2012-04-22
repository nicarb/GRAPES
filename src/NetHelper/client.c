#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

#include "client.h"
#include "utils.h"
#include "sockaddr-helpers.h"
#include "poll-send.h"
#include "poll-recv.h"
#include "timeout.h"

static const unsigned CLIENT_TIMEOUT_MINUTES = 10;

struct client {
    poll_send_t send;
    poll_recv_t recv;

    int epollfd;
    int fd;

    tout_t tout;
    sockaddr_t addr;

    unsigned broken : 1;
    unsigned enqueued : 1;
};

static int tcp_connect (const sockaddr_t *to);

client_t client_new (const sockaddr_t *addr)
{
    client_t ret;
    struct timeval max_tout;

    ret = mem_new(sizeof(struct client));

    ret->send = NULL;
    ret->recv = NULL;

    ret->fd = -1;
    ret->epollfd = -1;

    sockaddr_copy(&ret->addr, addr);
    max_tout.tv_sec = CLIENT_TIMEOUT_MINUTES * 60;
    max_tout.tv_usec = 0;
    ret->tout = tout_new(&max_tout);

    ret->broken = 0;
    ret->enqueued = 0;

    return ret;
}

void client_reset (client_t cl)
{
    if (cl->fd != -1) close(cl->fd);
    cl->fd = -1;
    cl->epollfd = -1;

    poll_recv_del(cl->recv);
    cl->recv = NULL;
    poll_send_del(cl->send);
    cl->send = NULL;

    cl->broken = 0;
    cl->enqueued = 0;
}

int client_connect (client_t cl, const sockaddr_t *to, int epollfd)
{
    int fd = tcp_connect(to);

    if (fd == -1) return -1;
    return client_set_fd(cl, fd, epollfd);
}

const sockaddr_t * client_get_addr (client_t cl)
{
    return &cl->addr;
}

int client_set_fd (client_t cl, int fd, int epollfd)
{
    poll_send_del(cl->send);
    poll_recv_del(cl->recv);

    if (cl->fd != -1) close(cl->fd);
    cl->fd = fd;
    cl->epollfd = epollfd;

    cl->send = poll_send_new(fd, epollfd);
    cl->recv = poll_recv_new(fd, epollfd);

    if (cl->send == NULL || cl->recv == NULL) {
        cl->broken = 1;
        return -1;
    }

    tout_reset(cl->tout);

    return 0;
}

int client_send_hello (client_t cl, const sockaddr_t *local_srv)
{
    assert(cl->fd != -1);
    return sockaddr_send_hello(local_srv, cl->fd);
}

int client_valid (client_t cl)
{
    if (cl == NULL) return 0;

    /* NOTE:
        The client is alive as long as both the poll-send and the
        poll-recv are alive. However a poll-recv instance remains alive
        until holding a message even if its file descriptor is broken.
        Thus we need to check it before its poll-send counterpart.

        If this is done a broken poll-recv carrying a message could be
        deallocated by the dictionary-based garbage collector, hence
        making the message queue referencing a deallocated memory area.

        UPDATE:
        There's still need to track if the element is enqueued or not,
        thus the `enqueued` flag. Still the invariant MUST hold.
     */

    if (cl->enqueued) {
        assert(poll_recv_is_alive(cl->recv));
        return 1;
    }

    return fcntl(cl->fd, F_GETFD) != -1 &&  // valid fd
           poll_recv_is_alive(cl->recv) &&
           poll_send_is_alive(cl->send) &&
           !(tout_expired(cl->tout) || cl->broken);
}

const msg_buf_t * client_read (client_t cl)
{
    const msg_buf_t *ret;

    switch (poll_recv_retrieve(cl->recv, &ret)) {
        case POLL_RECV_FAIL:
            cl->broken = 1;
        case POLL_RECV_BUSY:
        case POLL_RECV_STOP:
            return NULL;
        case POLL_RECV_SUCCESS:
        default:
            tout_reset(cl->tout);
            return ret;
    }
}

int client_has_message (client_t cl)
{
    return poll_recv_has_message(cl->recv);
}

int client_write (client_t cl, const msg_buf_t *msg)
{
    tout_reset(cl->tout);
    switch (poll_send_enqueue(cl->send, msg)) {
        case POLL_SEND_FAIL:
            cl->broken = 1;
        case POLL_SEND_BUSY:
            return -1;
        case POLL_SEND_SUCCESS:
        default:
            return 0;
    }
}

int client_flag_enqueued (client_t cl, int val)
{
    int ret;

    ret = cl->enqueued;
    cl->enqueued = val;
    return ret;
}

void client_del (client_t cl)
{
    if (cl == NULL) return;

    if (cl->fd != -1) close(cl->fd);
    poll_send_del(cl->send);
    poll_recv_del(cl->recv);
    tout_del(cl->tout);
    free(cl);
}

static
int tcp_connect (const sockaddr_t *to)
{
    int fd;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        print_err("Connecting", "socket", errno);
        return -1;
    }

    if (connect(fd, &to->sa, sockaddr_size(to)) == -1) {
        print_err("Connecting", "connect", errno);
        close(fd);
        return -1;
    }

    return fd;
}

