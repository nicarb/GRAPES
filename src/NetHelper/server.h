#ifndef SERVER_H
#define SERVER_H

#include "dictionary.h"
#include "sockaddr-helpers.h"

typedef struct server * server_t;

server_t server_new (const sockaddr_t *addr, int backlog,
                     int epollfd, dict_t neighbors);

void server_del (server_t);

#endif // SERVER_H

