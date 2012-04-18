#include <unistd.h>
#include <assert.h>

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
    tout_t tout;
    struct sockaddr_storage addr;

    unsigned broken : 1;
    unsigned enqueued : 1;
};

client_t client_new (int clientfd, int epollfd,
                     const struct sockaddr *addr)
{
    client_t ret;
    struct timeval max_tout;

    ret = mem_new(sizeof(struct client));
    ret->send = NULL;
    ret->recv = NULL;
    ret->epollfd = epollfd;
    sockaddr_copy((struct sockaddr *)&ret->addr, addr);
    ret->broken = 0;

    max_tout.tv_sec = CLIENT_TIMEOUT_MINUTES * 60;
    max_tout.tv_usec = 0;
    ret->tout = tout_new(&max_tout);

    client_setfd(ret, clientfd);

    return ret;
}

const struct sockaddr * client_get_addr (client_t cl)
{
    return (const struct sockaddr *) &cl->addr;
}

void client_setfd (client_t cl, int newfd)
{
    poll_send_del(cl->send);
    poll_recv_del(cl->recv);

    cl->send = poll_send_new(newfd, cl->epollfd);
    cl->recv = poll_recv_new(newfd, cl->epollfd);
    close(newfd);   // it has been dup()licated, we can close it.
}

int client_valid (client_t cl)
{
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

    return poll_recv_is_alive(cl->recv) &&
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

    poll_send_del(cl->send);
    poll_recv_del(cl->recv);
    tout_del(cl->tout);
}

