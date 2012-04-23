#ifndef POLL_RECV_H
#define POLL_RECV_H

#include <stddef.h>
#include "nh-types.h"

typedef struct poll_recv * poll_recv_t;

typedef enum {
    POLL_RECV_FAIL = -1,
    POLL_RECV_SUCCESS = 0,
<<<<<<< HEAD
    POLL_RECV_BUSY = 1
} poll_recv_res_t;

/** Create a new poll
 *
 * @param[in] fd
 * @param[in] epollfd
 *
 * @return a new poll_recv data structure 
 *
 * @see 
 */
poll_recv_t poll_recv_new (int fd, int epollfd);

/** 
 *
 * @param[in] pr
 * @param[in] out
 *
 * @return 
 *
 * @see 
 */
poll_recv_res_t poll_recv_retrieve (poll_recv_t pr, const msg_buf_t **out);

/** Check if there are messages
 *
 * @param[in] pr
 *
 * @return 
 *
 * @see 
 */
int poll_recv_has_message (poll_recv_t pr);

/** Check if pr is still alive
 *
 * @param[in] pr
 *
 * @return 
 *
 * @see 
 */
int poll_recv_is_alive (poll_recv_t pr);

/** Delete ps
 *
 * @param[in] ps
 *
 * @see 
 */
=======
    POLL_RECV_BUSY,
    POLL_RECV_STOP
} poll_recv_res_t;

poll_recv_t poll_recv_new (int fd, int epollfd);

poll_recv_res_t poll_recv_retrieve (poll_recv_t pr, const msg_buf_t **out);

int poll_recv_has_message (poll_recv_t pr);

int poll_recv_is_alive (poll_recv_t pr);

>>>>>>> 032b4dc85bcc84143b81cc44170867d424eab1a7
void poll_recv_del (poll_recv_t ps);

#endif // POLL_RECV_H

