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

static int tcp_serve (const struct sockaddr *srv, int backlog);
static void * poll_callback (void *context, int fd, int epollfd);
static int update_neighbors (dict_t neighbors, int clnfd, int epollfd);

server_t server_new (const struct sockaddr *addr, int backlog,
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
int tcp_serve (const struct sockaddr *srv, int backlog)
{
    int fd;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        print_err("Creating server", "socket", errno);
        return -1;
    }

    if (bind(fd, srv, sockaddr_size(srv)) == -1) {
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
    struct sockaddr_storage remote;
    dict_data_t record;
    client_t *client;

    if ((sockaddr_recv_hello((struct sockaddr *) &remote, clnfd)) == -1) {
        return -1;
    }

    record = dict_search(neighbors, (struct sockaddr *) &remote);
    client = (client_t *) dict_data_user(record);

    if (*client == NULL) {
        *client = client_new(clnfd, epollfd);
        // TODO: *client == NULL ?
        // also: update via client_setfd() ?
    } else {
        dict_data_update(record);
    }

    return 0;
}

static
void * poll_callback (void *context, int srvfd, int epollfd)
{
    server_t srv;
    struct sockaddr_storage addr;
    socklen_t addrlen;
    int clnfd;

    srv = (server_t) context;
    addrlen = sizeof(struct sockaddr_storage);
    clnfd = accept(srvfd, (struct sockaddr *) &addr, &addrlen);
    if (clnfd == -1) {
        print_err("Adding connection", "accept", errno);
    } else if (update_neighbors(srv->neighbors, clnfd, epollfd) == -1) {
        close(clnfd);
    }

    return context;
}

