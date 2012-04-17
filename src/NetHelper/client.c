#include "client.h"

struct client {
    poll_send_t send;
    poll_recv_t recv;
    int epollfd;
};

client_t client_new (int clientfd, int epollfd)
{
}

void client_setfd (client_t, int newfd)
{
}

int client_valid (client_t)
{
}

void client_del (client_t cl)
{
    if (cl == NULL) return;
}

