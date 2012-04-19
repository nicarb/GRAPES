/*
 *  Copyright (c) 2012 Arber Fama,
 *                     Giovanni Simoni
 *
 *  This is free software; see lgpl-2.1.txt
 */

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/select.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>

#include "net_helper.h"
#include "config.h"

#include "NetHelper/sockaddr-helpers.h"
#include "NetHelper/utils.h"
#include "NetHelper/server.h"
#include "NetHelper/poll-cb.h"
#include "NetHelper/client.h"
#include "NetHelper/inbox.h"
#include "NetHelper/timeout.h"

/* -- Internal data types -------------------------------------------- */

typedef struct {
    unsigned refcount;          // Reference counter (workaround for node
                                // copying).
    int pollfd;
    server_t server;
    dict_t neighbors;
    inbox_t inbox;

} local_info_t;

typedef struct nodeID {
    struct sockaddr_storage addr;   // local addr (pointed by paddr)
    struct sockaddr * paddr;        // convenience pointer to addr
    local_info_t *local;            // non-NULL only for local node

    struct {
        char ip[INET_ADDRSTRLEN];           // ip address
        char ip_port[INET_ADDRSTRLEN + 6];  // ip:port 2^16 -> 5 cyphers.
    } repr;                         // reentrant string representations
} nodeid_t;

typedef struct {
    int *user_fds;
    unsigned n_user_fds;
    int wakeup;
    pollcb_t cb;
    int allfd;
} user_poll_t;

/* -- Internal functions --------------------------------------------- */

static local_info_t * local_new (struct sockaddr *addr, struct tag *cfg);
static void local_del (local_info_t *l);
static int local_run_epoll (local_info_t *l, int toutmilli);

/* -- Internal functions for user file descriptors ------------------- */
static void * user_poll_callback (void *ctx, int uefd, int epollfd);
static int user_poll_init (user_poll_t *upoll, int *user_fds, int pollfd);
static void user_poll_clear (user_poll_t *upoll);

/* -- Constants ------------------------------------------------------ */

static const size_t MAX_CHECKED_EVENTS = 32;

static const unsigned DEFAULT_BACKLOG = 5;
static const char CONF_KEY_BACKLOG[] = "TCPBacklog";

/* -- Interface exported symbols ------------------------------------- */

struct nodeID *nodeid_dup (struct nodeID *s)
{
    nodeid_t *ret;

    if (s->local == NULL) {
        ret = malloc(sizeof(nodeid_t));
        if (ret == NULL) return NULL;
        memcpy(ret, s, sizeof(nodeid_t));
    } else {
        /* This reference counter trick will avoid copying around of the
         * nodeid for the local host */
        s->local->refcount ++;
        ret = s;
    }

    return ret;
}

/*
* @return -1, 0 or 1, depending on the relation between  s1 and s2.
*/
int nodeid_cmp (const nodeid_t *s1, const nodeid_t *s2)
{
    return sockaddr_cmp(s1->paddr, s2->paddr);
}

/* @return 1 if the two nodeID are identical or 0 if they are not. */
int nodeid_equal (const nodeid_t *s1, const nodeid_t *s2)
{
    return sockaddr_equal(s1->paddr, s2->paddr);
}

struct nodeID *create_node (const char *IPaddr, int port)
{
    nodeid_t *ret;

    ret = mem_new(sizeof(nodeid_t));
    ret->paddr = (struct sockaddr *) &ret->addr;
    ret->local = NULL;

    /* Initialization of address:
     *
     *      Note that this works only for ipv4. The internal functions in
     *      sockaddr-helpers, however, should be ipv6-ready.
     */
    if (sockaddr_in_init((struct sockaddr_in *)ret->paddr,
                         IPaddr, port) == -1) {
        free(ret);
        return NULL;
    }

    /* String representation (this part will be updated in the reentrant
     * branch of GRAPES) */
    sockaddr_strrep((struct sockaddr *)ret->paddr, ret->repr.ip,
                    INET_ADDRSTRLEN);
    sprintf(ret->repr.ip_port, "%s:%hu", ret->repr.ip, (uint16_t)port);

    return ret;
}

void nodeid_free (struct nodeID *s)
{
    if (s->local) local_del(s->local);
    free(s);
}

struct nodeID * net_helper_init (const char *IPaddr, int port,
                                 const char *config)
{
    nodeid_t *self;
    struct tag *cfg;

    self = create_node(IPaddr, port);
    if (self == NULL) {
        return NULL;
    }

    cfg = config_parse(config);
    if ((self->local = local_new(self->paddr, cfg)) == NULL) {
        free(cfg);
        nodeid_free(self);
        return NULL;
    }
    free(cfg);

    return self;
}

void bind_msg_type(uint8_t msgtype) {}

int send_to_peer(const struct nodeID *self, struct nodeID *to,
                 const uint8_t *buffer_ptr, int buffer_size)
{
    return 0;
}

int recv_from_peer(const struct nodeID *self, struct nodeID **remote,
                   uint8_t *buffer_ptr, int buffer_size)
{
    local_info_t *local;
    client_t from;
    const struct sockaddr *fromaddr;
    const msg_buf_t *msg;
    int size;

    assert(self->local != NULL);
    local = self->local;

    while (inbox_empty(local->inbox)) {
        if (local_run_epoll(local, -1)) {
            return -1;
        }
        inbox_scan_dict(local->inbox, local->neighbors);
    }

    from = inbox_next(local->inbox);
    msg = client_read(from);
    if (msg == NULL) {
        print_err("Retrieving message", NULL, EBADFD);
        return -1;
    }

    if (msg->size < buffer_size) {
        print_err("Retrieving message", NULL, ENOBUFS);
        return -1;
    }

    fromaddr = client_get_addr(from);
    size = (int) sockaddr_size(fromaddr);
    *remote = nodeid_undump((const uint8_t *) fromaddr, &size);
    assert(*remote != NULL);    // NULL here makes no sense!

    memcpy((void *)buffer_ptr, msg->data, msg->size);
    return msg->size;
}

int wait4data(const struct nodeID *self, struct timeval *tout,
              int *user_fds)
{
    unsigned now, time_limit;
    local_info_t *local;
    user_poll_t user_poll;

    assert(self->local != NULL);

    local = self->local;
    if (user_poll_init(&user_poll, user_fds, local->pollfd) == -1) {
        return -1;
    }
    time_limit = tout_now_ms() + tout_timeval_to_ms(tout);

    while (inbox_empty(local->inbox) && !user_poll.wakeup &&
           (now = tout_now_ms()) < time_limit) {
        if (local_run_epoll(local, time_limit - now) == -1) {
            user_poll_clear(&user_poll);
            return -1;
        }
        inbox_scan_dict(local->inbox, local->neighbors);
    }
    user_poll_clear(&user_poll);

    return (inbox_empty(local->inbox) && !user_poll.wakeup) ? 0 : 1;
}

struct nodeID *nodeid_undump (const uint8_t *b, int *len)
{
    nodeid_t *ret;

    ret = create_node(NULL, 0);
    if (sockaddr_undump(ret->paddr, *len, (const void *)b) == -1) {
        return NULL;
    }

    /* String representation (this part will be updated in the reentrant
     * branch of GRAPES) */
    sockaddr_strrep((struct sockaddr *)ret->paddr, ret->repr.ip,
                    INET_ADDRSTRLEN);
    sprintf(ret->repr.ip_port, "%s:%hu", ret->repr.ip,
            (uint16_t) sockaddr_port(ret->paddr));

    *len = sockaddr_size(ret->paddr);
    return ret;
}

int nodeid_dump (uint8_t *b, const struct nodeID *s,
                 size_t max_write_size)
{
    assert(s->local == NULL);
    return sockaddr_dump((void *)b, max_write_size, s->paddr);
}

const char *node_ip(const struct nodeID *s)
{
    return s->repr.ip;
}

const char *node_addr (const struct nodeID *s)
{
    return s->repr.ip_port;
}

/* -- Internal functions --------------------------------------------- */

static
local_info_t * local_new (struct sockaddr *addr, struct tag *cfg)
{
    local_info_t * L;
    int tcp_backlog;

    L = mem_new(sizeof(local_info_t));
    L->refcount = 1;
    L->pollfd = -1;
    L->server = NULL;
    L->neighbors = NULL;
    L->inbox = NULL;

    if ((L->pollfd = epoll_create1(0)) == -1) {
        print_err("Net-helper (tcp)", "epoll_create", errno);
        return NULL;
    }

    tcp_backlog = DEFAULT_BACKLOG;
    if (cfg) {
        config_value_int_default(cfg, CONF_KEY_BACKLOG, &tcp_backlog,
                                 DEFAULT_BACKLOG);
    }

    if ((L->neighbors = dict_new(cfg, (dict_delcb_t) client_del,
                                 (dict_pred_t) client_valid)) == NULL) {
        local_del(L);
        return NULL;
    }
    L->inbox = inbox_new();
    if ((L->server = server_new(addr, tcp_backlog, L->pollfd,
                                L->neighbors)) == NULL) {
        local_del(L);
        return NULL;
    }

    return L;
}

static
void local_del (local_info_t *L)
{
    if (-- L->refcount > 0) return;

    close(L->pollfd);
    server_del(L->server);
    dict_del(L->neighbors);
    inbox_del(L->inbox);
    free(L);
}

static
int local_run_epoll (local_info_t *l, int toutmilli)
{
    struct epoll_event events[MAX_CHECKED_EVENTS];
    int i, nev;

    nev = epoll_wait(l->pollfd, events, MAX_CHECKED_EVENTS, toutmilli);
    if (nev == -1) {
        print_err("Polling", "epoll_wait", errno);
        return -1;
    }

    for (i = 0; i < nev; i ++) {
        pollcb_run((pollcb_t) events[i].data.ptr);
    }

    return 0;
}

static
void * user_poll_callback (void *ctx, int uefd, int epollfd)
{
    user_poll_t * upoll = ctx;
    int i, N;
    struct epoll_event events[upoll->n_user_fds];

    upoll->wakeup = 1;

    N = epoll_wait(uefd, events, upoll->n_user_fds, 0);
    for (i = 0; i < N; i ++) {
        int *ufd = (int *)events[i].data.ptr;
        *ufd = -2;  // According to original versions of net_helper-udp
    }

    return ctx;
}

static
int user_poll_init (user_poll_t *upoll, int *user_fds, int pollfd)
{
    int fd;
    unsigned i;

    upoll->user_fds = user_fds;
    upoll->wakeup = 0;
    upoll->cb = NULL;

    fd = epoll_create1(0);
    if (fd == -1) {
        print_err("Net-helper", "epoll_create", errno);
        return -1;
    }
    upoll->allfd = fd;
    for (i = 0; user_fds[i] != -1; i ++) {
        struct epoll_event ev;

        ev.events = EPOLLIN;
        ev.data.ptr = (void *) &user_fds[i];
        if (epoll_ctl(fd, EPOLL_CTL_ADD, user_fds[i], &ev) == -1) {
            user_poll_clear(upoll);
            return -1;
        }
    }
    upoll->n_user_fds = i;
    upoll->cb = pollcb_new(user_poll_callback, (void *)upoll,
                           upoll->allfd, pollfd);
    if (pollcb_enable(upoll->cb, EPOLLIN) == -1) {
        user_poll_clear(upoll);
        return -1;
    }

    return 0;
}

static
void user_poll_clear (user_poll_t *upoll)
{
    if (upoll->cb != NULL) {
        pollcb_disable(upoll->cb);
        pollcb_del(upoll->cb);
    }
    close(upoll->allfd);
}
