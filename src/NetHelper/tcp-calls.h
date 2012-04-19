#ifndef TCP_CALLS_H
#define TCP_CALLS_H

/* Both return -1 on error and fd on success */

/** 
 *
 * @param[in] 
 *
 * @return 
 *
 * @see 
 */
int tcp_serve (const struct sockaddr *srv, int backlog);

/** 
 *
 * @param[in] 
 *
 * @return 
 *
 * @see 
 */
int tcp_connect (const struct sockaddr *to);

#endif // TCP_CALLS_H

