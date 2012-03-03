/*
 *  Copyright (c) 2012 Arber Fama,
 *                     Giovanni Simoni
 *
 *  This is free software; see lgpl-2.1.txt
 */

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include "net_helper.h"
#include "config.h"

/* -- Internal functions ---------------------------------------------- */

static int tcp_connect (sock_data_t *sd, int *e);
static int tcp_serve (sock_data_t *sd, int backlog, int *e);

/* -- Constants ------------------------------------------------------- */

static const char *   CONF_KEY_BACKLOG = "tcp_backlog";
static const unsigned DEFAULT_BACKLOG = 50;

/* -- Interface exported symbols -------------------------------------- */

struct nodeID {
    struct sockaddr_in addr;
    int fd;
    struct {
        unsigned self : 1;
    } conf;
};

typedef struct nodeID nodeid_t;

struct nodeID *nodeid_dup (struct nodeID *s)
{
    nodeid_t *ret = malloc(sizeof(nodeid_t));

    if (ret == NULL) return NULL;
    memcpy(ret, s, sizeof(nodeid_dup));
    return ret;
}

/*
* @return -1, 0 or 1, depending on the relation between  s1 and s2.
*/
int nodeid_cmp (const struct nodeID *s1, const struct nodeID *s2)
{
    return memcmp(&s1->addr, &s2->addr, sizeof(struct sockaddr_in));
}


/* @return 1 if the two nodeID are identical or 0 if they are not. */
int nodeid_equal (const struct nodeID *s1, const struct nodeID *s2)
{
    return nodeid_cmp(&s1, &s2) == 0;
}


struct nodeID *create_node (const char *IPaddr, int port)
{
    nodeid_t *ret = malloc(sizeof(nodeid_t));

    ret->addr.sin_family = AF_INET;
    ret->addr.sin_port = htons(port);
    ret->fd = -1;

    if (IPaddr == NULL) {
        ret->addr.sin_addr = INADDR_ANY;
    }

    if (inet_aton(IPaddr, &ret->addr.sin_addr) == 0) {
        free(ret);
        return NULL;
    }

    ret->conf.self = 0;
    return ret;
}

void nodeid_free (struct nodeID *s)
{
    int fd;

    if ((fd = s->fd) == -1) {
        close(fd);
    }
    free(s);
}

struct nodeID * net_helper_init (const char *IPaddr, int port,
                                 const char *config)
{
    nodeid_t *this;
    struct tag *cfg_tags;
    int backlog;
    int e;

    this = create_node(IPaddr, port);
    if (this == NULL) return NULL;
    this->conf.self = 1;

    backlog = DEFAULT_BACKLOG;
    if (config && (cfg_tags = config_parse(config))) {
        config_value_int_default(cfg_tags, CONF_KEY_BACKLOG, &backlog,
                                 DEFAULT_BACKLOG);
        free(cfg_tags);
        fprintf(stderr, "Backlog change\n");
    }
    fprintf(stderr, "Backlog value: %i\n", backlog);

    if (tcp_serve(&this, backlog, &e) < 0) {
        fprintf(stderr, "net-helper: creating server errno %d: %s\n", e,
                strerror(e));
        nodeid_free(this);
        return NULL;
    }

    return this;
}

void bind_msg_type(uint8_t msgtype)
{
    /* Noy yet developed! */
}

int send_to_peer(const struct nodeID *from, struct nodeID *to,
                 const uint8_t *buffer_ptr, int buffer_size)
{
  /* Noy yet developed! */
  return -1;
}

int recv_from_peer(const struct nodeID *local, struct nodeID **remote,
                   uint8_t *buffer_ptr, int buffer_size)
{
  /* Noy yet developed! */
  return -1;
}

int wait4data(const struct nodeID *n, struct timeval *tout, int *user_fds)
{
  /* Noy yet developed! */
  return -1;
}

const char *node_addr(const struct nodeID *s)
{
  /* Noy yet developed! */
  static char addr[256];

  sprintf(addr, "%s:%d", inet_ntoa(s->addr.sin_addr), ntohs(s->addr.sin_port));

  return addr;
}

struct nodeID *nodeid_undump(const uint8_t *b, int *len)
{
  struct nodeID *res;

  return res;
}

int nodeid_dump(uint8_t *b, const struct nodeID *s, size_t max_write_size)
{
}

const char *node_ip(const struct nodeID *s)
{
  
}

/* -- Internal functions ---------------------------------------------- */

static
int tcp_connect (nodeID *sd, int *e)
{
    int fd;

    if (sd->fd != -1) {
        /* Already connected */
        return 0;
    }

    fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd == -1) {
        if (e) *e = errno;
        return -1;
    }

    if (connect(fd, (struct sockaddr *) &sd->addr,
            sizeof(struct sockaddr_in)) == -1) {
        if (e) *e = errno;
        close(fd);
        return -2;
    }
    sd->fd = fd;

    return 0;
}

static
int tcp_serve (nodeID *sd, int backlog, int *e)
{
    int fd;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        if (e) *e = errno;
        return -1;
    }

    if (bind(fd, (struct sockaddr *) &sd->addr,
            sizeof(struct sockaddr_in))) {
        if (e) *e = errno;
        close(fd);
        return -2;
    }

    if (listen(fd, backlog) == -1) {
        if (e) *e = errno;
        close(fd);
        return -3;
    }
    sd->fd = fd;

    return 0;
}

