/*
 *  Copyright (c) 2012 Arber Fama,
 *                     Giovanni Simoni
 *
 *  This is free software; see lgpl-2.1.txt
 */

#include <netinet/in.h>

#include "net_helper.h"
#include "config.h"

#include "NetHelper/sockaddr-helpers.h"
#include "NetHelper/utils.h"

/* -- Internal data types -------------------------------------------- */

typedef struct {
    unsigned refcount;          // Reference counter (workaround for node
                                // copying).
} local_info_t;

typedef struct nodeID {
    struct sockaddr_storage addr;
    struct sockaddr * paddr;
    local_info_t *local;            // non-NULL only for local node

    struct {
        char ip[INET_ADDRSTRLEN];           // ip address
        char ip_port[INET_ADDRSTRLEN + 6];  // ip:port 2^16 -> 5 cyphers.
    } repr;                         // reentrant string representations
} nodeid_t;

/* -- Internal functions --------------------------------------------- */

static local_info_t * local_new (const char *config);
static void local_del (local_info_t *l);

/* -- Constants ------------------------------------------------------ */

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
    inet_ntop(AF_INET, (const void *)&ret->addr.sin_addr,
              ret->repr.ip, INET_ADDRSTRLEN);
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
    struct tag *cfg_tags;
    local_info_t *local;

    self = create_node(IPaddr, port);
    if (self == NULL) return NULL;
    self->local = local_new(config);

    return self;
}

void bind_msg_type(uint8_t msgtype) {}

int send_to_peer(const struct nodeID *self, struct nodeID *to,
                 const uint8_t *buffer_ptr, int buffer_size)
{
}

int recv_from_peer(const struct nodeID *self, struct nodeID **remote,
                   uint8_t *buffer_ptr, int buffer_size)
{
}

int wait4data(const struct nodeID *self, struct timeval *tout,
              int *user_fds)
{
}

struct nodeID *nodeid_undump (const uint8_t *b, int *len)
{
    /* TODO: complete. See sockaddr_undump */
}

int nodeid_dump (uint8_t *b, const struct nodeID *s,
                 size_t max_write_size)
{
    /* TODO: complete. See sockaddr_dump */
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

static local_info_t * local_new (const char *config)
{
    local_info_t * local = mem_new(sizeof(local_info_t));
    local->refcount = 1;

    return local;
}

static void local_del (local_info_t *l)
{
    if (-- l->refcount == 0) {
        free(l);
    }
}

