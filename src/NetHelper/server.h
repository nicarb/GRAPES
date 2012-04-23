#ifndef SERVER_H
#define SERVER_H

#include "dictionary.h"
<<<<<<< HEAD

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
=======
#include "sockaddr-helpers.h"

typedef struct server * server_t;

server_t server_new (const sockaddr_t *addr, int backlog,
                     int epollfd, dict_t neighbors);

>>>>>>> 032b4dc85bcc84143b81cc44170867d424eab1a7
void server_del (server_t);

#endif // SERVER_H

