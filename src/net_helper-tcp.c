/*
 *  Copyright (c) 2012 Arber Fama,
 *                     Giovanni Simoni
 *
 *  This is free software; see lgpl-2.1.txt
 */


#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "net_helper.h"
#include "config.h"
#include "NetHelper/dictionary.h"

/* -- Internal functions --------------------------------------------- */

static int tcp_connect (nodeID *sd, int *e);
static int tcp_serve (nodeID *sd, int backlog, int *e);

/* -- Constants ------------------------------------------------------ */

static const char *   CONF_KEY_BACKLOG = "tcp_backlog";
static const unsigned DEFAULT_BACKLOG = 50;

/* -- Interface exported symbols ------------------------------------- */

typedef struct {
	int fd;
	dict_t neighbors;
} local_info_t;

typedef struct nodeID {
	struct sockaddr_in addr;
	local_info_t *local;        // non-NULL only for local node */
} nodeid_t;

struct nodeID *nodeid_dup (struct nodeID *s)
{
	/* Local nodeID cannot be duplicated! */
	assert(s->local == NULL);

	nodeid_t *ret = malloc(sizeof(nodeid_t));

	if (ret == NULL) return NULL;
	memcpy(ret, s, sizeof(nodeid_t));
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

	if (IPaddr == NULL) {
		/* In case of server, specifying NULL will allow anyone to
		 * connect. */
		ret->addr.sin_addr = INADDR_ANY;
	} else if (inet_pton(AF_INET, IPaddr, (void *)&ret->addr.sin_addr) == 0) {
		fprintf(stderr, "Invalid ip address %s\n", IPaddr);
		free(ret);
		return NULL;
	}
	return ret;
}

void nodeid_free (struct nodeID *s)
{
	if (s->local != NULL) {
		/* TODO: dictionary delete should close also internal file
		 * descriptors */
		dict_delete(s->local.neighbors);
		close(s->local.fd);
		free(s->local);
	}
	free(s);
}

struct nodeID * net_helper_init (const char *IPaddr, int port,
								 const char *config)
{
	nodeid_t *this;
	struct tag *cfg_tags = NULL;
	int e;

	this = create_node(IPaddr, port);
	if (this == NULL) {
		return NULL;
	}

	if (config && (cfg_tags = config_parse(config))) {
		config_value_int_default(cfg_tags, CONF_KEY_BACKLOG, &backlog,
								 DEFAULT_BACKLOG);
	}

	if ((this->local = malloc(sizeof(local_info_t))) == NULL) {
		nodeid_free(this);
		free(cfg_tags);
		return NULL;
	}
	this->local.neighbors = dict_new(AF_INET, cfg_tags);
	free(cfg_tags);

	if (tcp_serve(&this, backlog, &e) < 0) {
		fprintf(stderr, "net-helper: creating server errno %d: %s\n", e,
				strerror(e));
		nodeid_free(this);
		return NULL;
	}

	return this;
}

void bind_msg_type(uint8_t msgtype) {}

/*
 */
int send_to_peer(const struct nodeID *from, struct nodeID *to,
				 const uint8_t *buffer_ptr, int buffer_size)
{
    int peer_fd;
    int check;
    int res = -1;

    assert(from->local != NULL);    // FIXME: is "from" the self node?
    if (dict_lookup(from->local->neighbors, &to->addr, &peer_fd) == -1) {
        /* TODO: here connect + insert into hash table the obtained
         * file-descriptor.
         * Is this good? Should we be aware of something here? */
    }

    /* TODO: then we can go with transmission */
#if 0
	if (to->fd > 0) {
		res = write(to->fd, &buffer_ptr, buffer_size);
	} else {
		fprintf(stderr, "net-helper-tcp: send_to_peer failed. Uncorrect file descriptor: %d.\n", to->fd);
	}

	if (res  < 0) {
		int error = errno;
		fprintf(stderr,"net-helper-tcp: sendmsg failed errno %d: %s\n",
				error, strerror(error));
	}
#endif

    return res;
}

int recv_from_peer(const struct nodeID *self, struct nodeID **remote,
				   uint8_t *buffer_ptr, int buffer_size)
{
	int res = -1;
    int peer_fd;
    struct sockaddr_in peer_addr;
    dict_t neigbors;

    assert(self->local != NULL);
    neighbors = self->local->neighbors;

    /* This is what external software using the interface is expecting */
	*remote = malloc(sizeof(struct nodeID));
	if (*remote == NULL) {
		return -1;
	}


    /* TODO: implement fair_select
     *
     * The interface should be something like:
     *
     * @param[in] hash The hash table;
     * @param[out] fd  The file descriptor of the peer which sent the
     *                 data;
     * @param[out] addr The corresponding address.
     */
#if 0
    fair_select(neighbors, &peer_fd, &peer_addr); // TODO: to be implemented
    res = read(fd, buffer, buffer_size);
#endif

	if (res <= 0) {
        /* As the socket is in error or EOF has been reached, this
         * connection must be closed */
        dict_remove(neighbors, &peer_addr);
		return res;
	}

	memcpy(&(*remote)->addr, &raddr, msg.msg_namelen);
	(*remote)->fd = -1;

	return res;
}

struct select_helper {
	fd_set readfds;
	int max;
};

static
int scan_fill_set (void *ctx, const struct sockaddr *addr, int fd)
{
	struct select_helper *sh = (struct select_helper *) ctx;
	FD_SET(fd, &sh->readfds);
	if (fd > sh->max) {
		sh->max = fd;
	}
	return 1;
}

/* Behaves in a way which is compatible with the `wait4data` function
 * provided by the UDP version */
int wait4data(const struct nodeID *n, struct timeval *tout,
			  int *user_fds)
{
	dict_t neighbors = n->local->neighbors;
	int res;
	struct select_helper sh;
	int res;

	/* We start with the file neighbors descriptors */

	FD_ZERO(&sh.readfds);
	sh.max = -1;
	dict_scan(neighbors, scan_fill_set, (void *)&sh);

	res = select(sh.max, &sh.readfds, NULL, NULL, tout);
	if (res < 0) {
		return res;         // We had an error.
	} else if (res > 0) {
		return 1;           // Incoming data from neighbors;
	}

	if (user_fds == NULL) {
		return 0;           // No incoming data and no user_fds to check
	}

	/* There may be some user file descriptors waiting. Starting again on
	 * user_fds */

	FD_ZERO(&sh.readfds);
	sh.max = -1;
	for (i = 0; user_fds[i] != -1; i++) {
		FD_SET(user_fds[i], &sh.readfds);
		if (user_fds[i] > sh.max) {
			sh.max = user_fds[i];
		}
	}
	res = select(sh.max, &sh.readfds, NULL, NULL, tout);
	if (res <= 0) {
		return res;
	}

	for (i = 0; user_fds[i] != -1; i++) {
		if (!FD_ISSET(user_fds[i], &fds)) {
			user_fds[i] = -2;
		}
	}

	return 2;
}

const char *node_addr(const struct nodeID *s)
{
	/* Noy yet developed! */
	static char addr[256];

	sprintf(addr, "%s:%d", inet_ntoa(s->addr.sin_addr),
			ntohs(s->addr.sin_port));

	return addr;
}

struct nodeID *nodeid_undump(const uint8_t *b, int *len) {
	struct nodeID *res;
	/* nothing done at the moment! */
	return res;
}

int nodeid_dump(uint8_t *b, const struct nodeID *s,
				size_t max_write_size)
{
}

const char *node_ip(const struct nodeID *s)
{
	return "";
}

/* -- Internal functions --------------------------------------------- */

static
int tcp_connect (nodeID *sd, int *e)
{
	int fd;
	/* TODO: here modify to support hash picking. It won't work till then. */

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
	assert(sd->local != NULL);  // TODO: remove when it works.

	fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
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
	sd->local->fd = fd;

	return 0;
}

