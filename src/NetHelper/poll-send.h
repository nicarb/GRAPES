#ifndef POLL_SEND_H
#define POLL_SEND_H

#include <stddef.h>

#include "nh-types.h"

typedef struct poll_send * poll_send_t;

typedef enum {
    POLL_SEND_FAIL = -1,
    POLL_SEND_SUCCESS = 0,
<<<<<<< HEAD
    POLL_SEND_BUSY = 1
} poll_send_res_t;

/** Create a new poll
 *
 * @param[in] fd
 * @param[in] epollfd
 *
 * @return 
 *
 * @see 
 */
poll_send_t poll_send_new (int fd, int epollfd);

/** Enqueues message when it's ready for sending.
 *
 * @param[in] ps
 * @param[in] msg
 *
 * @return 
 *
 * @see 
 */
poll_send_res_t poll_send_enqueue (poll_send_t ps, const msg_buf_t *msg);

/** Check if poll is still alive...
 *
 * @param[in] ps
 *
 * @return 
 *
 * @see 
 */
int poll_send_is_alive (poll_send_t ps);

/** Delete the datastructure...
 *
 * @param[in] 
 *
 *
 * @see 
 */
=======
    POLL_SEND_BUSY,
    POLL_SEND_STOP
} poll_send_res_t;

poll_send_t poll_send_new (int fd, int epollfd);

poll_send_res_t poll_send_enqueue (poll_send_t ps, const msg_buf_t *msg);

int poll_send_is_alive (poll_send_t ps);

>>>>>>> 032b4dc85bcc84143b81cc44170867d424eab1a7
void poll_send_del (poll_send_t ps);

#endif // POLL_SEND_H

