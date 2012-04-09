#ifndef SERVER_H
#define SERVER_H

#include "dictionary.h"

typedef struct server * server_t;

server_t server_new (const struct sockaddr *addr, int backlog,
                     int epollfd, dict_t neighbors);

void server_del (server_t);

#endif // SERVER_H

