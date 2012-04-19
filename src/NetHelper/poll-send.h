#ifndef POLL_SEND_H
#define POLL_SEND_H

#include <stddef.h>

#include "nh-types.h"

typedef struct poll_send * poll_send_t;

typedef enum {
    POLL_SEND_FAIL = -1,
    POLL_SEND_SUCCESS = 0,
    POLL_SEND_BUSY = 1
} poll_send_res_t;

/** 
 *
 * @param[in] 
 *
 * @return 
 *
 * @see 
 */
poll_send_t poll_send_new (int fd, int epollfd);

/** 
 *
 * @param[in] 
 *
 * @return 
 *
 * @see 
 */
poll_send_res_t poll_send_enqueue (poll_send_t ps, const msg_buf_t *msg);

/** 
 *
 * @param[in] 
 *
 * @return 
 *
 * @see 
 */
int poll_send_is_alive (poll_send_t ps);

/** 
 *
 * @param[in] 
 *
 * @return 
 *
 * @see 
 */
void poll_send_del (poll_send_t ps);

#endif // POLL_SEND_H

