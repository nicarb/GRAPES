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

struct server {
    int fd;
    pollcb_t cb;
    dict_t neighbors;
};

static int tcp_serve (const struct sockaddr *srv, int backlog);
static void * poll_callback (void *context, int epollfd);
static void update_neighbors (dict_t neighbors, int newfd, int epollfd);

server_t server_new (const struct sockaddr *addr, int backlog,
                     int epollfd, dict_t neighbors)
{
    server_t srv;
    struct epoll_event ev;
    int fd;

    fd = tcp_serve(addr, backlog);
    if (fd == -1) return NULL;

    srv = mem_new(sizeof(struct server));
    srv->fd = fd;
    srv->cb = pollcb_new(poll_callback, (void *) srv);
    srv->neighbors = neighbors;

    ev.events = EPOLLIN;
    ev.data.ptr = srv->cb;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        print_err("Creating server", "epoll_ctl", errno);
        server_del(srv);
        return NULL;
    }

    return srv;
}

void server_del (server_t srv)
{
    close(srv->fd);
    pollcb_del(srv->cb);
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
void update_neighbors (dict_t neighbors, int newfd, int epollfd)
{
    /* TODO: receive real address (?); update neighbors; subscribe
     *       polling. */
}

static
void * poll_callback (void *context, int epollfd)
{
    server_t srv;
    struct sockaddr_storage addr;
    socklen_t addrlen;
    int fd;

    srv = (server_t) context;
    addrlen = sizeof(struct sockaddr_storage);
    fd = accept(srv->fd, (struct sockaddr *) &addr, &addrlen);
    if (fd == -1) {
        print_err("Adding connection", "accept", errno);
    } else {
        update_neighbors(srv->neighbors, fd, epollfd);
    }
    return context;
}

