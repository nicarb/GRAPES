#ifndef CLIENT_H
#define CLIENT_H

typedef struct client * client_t;

client_t client_new (int clientfd, int epollfd);

/* Change file descriptor if needed */
void client_setfd (client_t, int newfd);

/* For dictionary validity */
int client_valid (client_t);

void client_del (client_t);

#endif // CLIENT_H

