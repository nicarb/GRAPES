#ifndef TCP_CALLS_H
#define TCP_CALLS_H

/** 
 *
 * @param[in] srv
 * @param[in] backlog
 *
 * @return -1 on error or fd on success 
 *
 * @see
 */
int tcp_serve (const struct sockaddr *srv, int backlog);

/** Function for connecting to a target
 *
 * @param[in] to the target
 *
 * @return -1 on error or fd on success 
 *
 * @see 
 */
int tcp_connect (const struct sockaddr *to);

#endif // TCP_CALLS_H

