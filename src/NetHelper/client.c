#include <unistd.h>

#include "client.h"
#include "utils.h"
#include "poll-send.h"
#include "poll-recv.h"

struct client {
    poll_send_t send;
    poll_recv_t recv;
    int epollfd;

    unsigned broken : 1;
};

client_t client_new (int clientfd, int epollfd)
{
    client_t ret;

    ret = mem_new(sizeof(struct client));
    ret->send = NULL;
    ret->recv = NULL;
    ret->epollfd = epollfd;
    ret->broken = 0;

    client_setfd(ret, clientfd);

    return ret;
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
     */
    if (poll_recv_is_alive(cl->recv)) return 1;
    return poll_send_is_alive(cl->send) && !cl->broken;
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
            return ret;
    }
}

int client_has_message (client_t cl)
{
    return poll_recv_has_message(cl->recv);
}

int client_write (client_t cl, const msg_buf_t *msg)
{
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

void client_del (client_t cl)
{
    if (cl == NULL) return;

    poll_send_del(cl->send);
    poll_recv_del(cl->recv);
}

