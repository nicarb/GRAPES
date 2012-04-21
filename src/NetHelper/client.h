#ifndef CLIENT_H
#define CLIENT_H

#include "nh-types.h"
#include "sockaddr-helpers.h"

typedef struct client * client_t;

client_t client_new (const sockaddr_t *addr);

int client_connect (client_t cl, const sockaddr_t *to, int epollfd);

/* Change file descriptor if needed */
int client_set_fd (client_t cl, int clfd, int epollfd);

int client_send_hello (client_t cl, const sockaddr_t *local_srv);

void client_reset (client_t cl);

/* For dictionary validity */
int client_valid (client_t cl);

int client_has_message (client_t cl);

const msg_buf_t * client_read (client_t cl);

int client_write (client_t cl, const msg_buf_t *msg);

const sockaddr_t * client_get_addr (client_t cl);

int client_flag_enqueued (client_t cl, int val);

void client_del (client_t cl);

#endif // CLIENT_H

