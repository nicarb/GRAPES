#ifndef CLIENT_H
#define CLIENT_H

#include "nh-types.h"

typedef struct client * client_t;

/* When receiving */
client_t client_new (int clientfd, int epollfd,
                     const struct sockaddr *addr);

/* When connecting */
client_t client_new_connect (int epollfd, const struct sockaddr *to,
                             const struct sockaddr *local_srv);

/* Change file descriptor if needed */
void client_setfd (client_t cl, int newfd);

/* For dictionary validity */
int client_valid (client_t cl);

int client_has_message (client_t cl);

const msg_buf_t * client_read (client_t cl);

int client_write (client_t cl, const msg_buf_t *msg);

const struct sockaddr * client_get_addr (client_t cl);

int client_flag_enqueued (client_t cl, int val);

void client_del (client_t cl);

#endif // CLIENT_H

