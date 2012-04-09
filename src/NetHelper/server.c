#include "server.h"
#include "poll-cb.h"

#include <sys/epoll.h>

struct server {
    int fd;
    pollcb_t cb;
    dict_t neighbors;
};

static int tcp_serve (const struct sockaddr *srv, int backlog);
static void * poll_callback (void *context, int epollfd);

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
        print_err("Server", "epoll_ctl", errno);
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
void * poll_callback (void *context, int epollfd)
{
    server_t srv;

    srv = (server_t) context;

    /* TODO: accept() and register the file descriptor */

    return context;
}

