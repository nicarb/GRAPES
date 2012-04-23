#ifndef CLIENT_H
#define CLIENT_H

#include "nh-types.h"
<<<<<<< HEAD

typedef struct client * client_t;

/** Create a new client
 *
 * @param[in] clientfd
 * @param[in] epollfd
 * @param[in] addr
 *
 * @return 
 *
 * @see 
 */
client_t client_new (int clientfd, int epollfd,
                     const struct sockaddr *addr);

/** Set fd to the client. 
 * Change file descriptor if needed
 * 
 * @param[in] cl
 * @param[in] newfd
 *
 * @return 
 *
 * @see 
 */
void client_setfd (client_t cl, int newfd);


/** Check for dictionary validity of the client
 *
 * @param[in] cl
 *
 * @return 
 *
 * @see 
 */
int client_valid (client_t cl);

/** Check if client has got a complete message to deliver
 *
 * @param[in] cl
 *
 * @return 
 *
 * @see 
 */
int client_has_message (client_t cl);

/** Read the message
 *
 * @param[in] cl
 *
 * @return 
 *
 * @see 
 */
const msg_buf_t * client_read (client_t cl);

/** 
 *
 * @param[in] 
 *
 * @return 
 *
 * @see 
 */
int client_write (client_t cl, const msg_buf_t *msg);

/** 
 *
 * @param[in] 
 *
 * @return 
 *
 * @see 
 */
const struct sockaddr * client_get_addr (client_t cl);

/** 
 *
 * @param[in] 
 *
 * @return 
 *
 * @see 
 */
int client_flag_enqueued (client_t cl, int val);

/** 
 *
 * @param[in] 
 *
 * @return 
 *
 * @see 
 */
=======
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

>>>>>>> 032b4dc85bcc84143b81cc44170867d424eab1a7
void client_del (client_t cl);

#endif // CLIENT_H

