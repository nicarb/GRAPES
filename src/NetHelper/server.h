#ifndef SERVER_H
#define SERVER_H

typedef struct server * server_t;

server_t server_new (const struct sockaddr *addr, int backlog,
                     int epollfd, dict_t neighbors);

pollcb_t server_get_pollcb (server_t);

void server_del (server_t);

#endif // SERVER_H

