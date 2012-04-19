#ifndef SERVER_H
#define SERVER_H

#include "dictionary.h"

typedef struct server * server_t;

/** 
 *
 * @param[in] addr
 * @param[in] backlig
 * @param[in] epollfmd
 * @param[in] neighbours
 *
 * @return 
 *
 * @see 
 */
server_t server_new (const struct sockaddr *addr, int backlog,
                     int epollfd, dict_t neighbors);

/** 
 *
 * @param[in] server_t
 *
 *
 * @see 
 */
void server_del (server_t);

#endif // SERVER_H

