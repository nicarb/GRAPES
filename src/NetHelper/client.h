#ifndef CLIENT_H
#define CLIENT_H

#include "nh-types.h"

typedef struct client * client_t;

client_t client_new (int clientfd, int epollfd);

/* Change file descriptor if needed */
void client_setfd (client_t cl, int newfd);

/* For dictionary validity */
int client_valid (client_t cl);

const msg_buf_t * client_read (client_t cl);

int client_write (client_t cl, const msg_buf_t *msg);

void client_del (client_t cl);

#endif // CLIENT_H

