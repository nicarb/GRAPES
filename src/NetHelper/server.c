#include <sys/epoll.h>
#include <sys/socket.h>
#include <stddef.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include "poll-cb.h"
#include "utils.h"
#include "sockaddr-helpers.h"

#include "server.h"
#include "client.h"

struct server {
    pollcb_t cb;
    dict_t neighbors;
};

static int tcp_serve (const sockaddr_t *srv, int backlog);
static void * poll_callback (void *context, int fd, int epollfd);
static int update_neighbors (dict_t neighbors, int clnfd, int epollfd);

server_t server_new (const sockaddr_t *addr, int backlog,
                     int epollfd, dict_t neighbors)
{
    server_t srv;
    int fd;

    fd = tcp_serve(addr, backlog);
    if (fd == -1) return NULL;

    srv = mem_new(sizeof(struct server));
    srv->neighbors = neighbors;
    srv->cb = pollcb_new(poll_callback, (void *)srv, fd, epollfd);
    close(fd);  /* Note: fd is dup()licated inside pollcb_new */

    if (pollcb_enable(srv->cb, EPOLLIN) == -1) {
        print_err("Creating server", "epoll_ctl", errno);
        server_del(srv);
        return NULL;
    }

    return srv;
}

void server_del (server_t srv)
{
    if (srv == NULL) return;
    if (srv->cb != NULL) {
        pollcb_del(srv->cb);
    }
    free(srv);
}

static
int tcp_serve (const sockaddr_t *srv, int backlog)
{
    int fd;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        print_err("Creating server", "socket", errno);
        return -1;
    }

    if (bind(fd, &srv->sa, sockaddr_size(srv)) == -1) {
        print_err("Creating server", "bind", errno);
        close(fd);
        return -2;
    }

    if (listen(fd, backlog) == -1) {
        print_err("Creating server", "listen", errno);
        close(fd);
        return -3;
    }

    return fd;
}

static
int update_neighbors (dict_t neighbors, int clnfd, int epollfd)
{
    sockaddr_t remote;
    dict_data_t record;
    client_t *client;

    if ((sockaddr_recv_hello(&remote, clnfd)) == -1) {
        return -1;
    }

    record = dict_search(neighbors, &remote);
    client = (client_t *) dict_data_user(record);

    if (*client == NULL) {
        if ((*client = client_new(clnfd, epollfd, &remote)) == NULL) {
            return -1;
        }
    } else {
        /* I already know this guy, so he knows me. Why is he calling me
         * back again? Replace the client file descriptor just to avoid
         * surprises. This is a corner case and totally unlikely, but if
         * we stumble into it we'll have to wait a long period before the
         * timeout kills both connections. Why taking the risk? */
        client_setfd(*client, clnfd);
    }

    return 0;
}

static
void * poll_callback (void *context, int srvfd, int epollfd)
{
    server_t srv;
    sockaddr_t addr;
    socklen_t addrlen;
    int clnfd;

    srv = (server_t) context;
    addrlen = sizeof(socklen_t);
    clnfd = accept(srvfd, &addr.sa, &addrlen);
    if (clnfd == -1) {
        print_err("Adding connection", "accept", errno);
    } else if (update_neighbors(srv->neighbors, clnfd, epollfd) == -1) {
        close(clnfd);
    }

    return context;
}

