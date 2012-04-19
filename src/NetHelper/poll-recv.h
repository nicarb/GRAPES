#ifndef POLL_RECV_H
#define POLL_RECV_H

#include <stddef.h>
#include "nh-types.h"

typedef struct poll_recv * poll_recv_t;

typedef enum {
    POLL_RECV_FAIL = -1,
    POLL_RECV_SUCCESS = 0,
    POLL_RECV_BUSY = 1
} poll_recv_res_t;

/** 
 *
 * @param[in] 
 *
 * @return 
 *
 * @see 
 */
poll_recv_t poll_recv_new (int fd, int epollfd);

/** 
 *
 * @param[in] 
 *
 * @return 
 *
 * @see 
 */
poll_recv_res_t poll_recv_retrieve (poll_recv_t pr, const msg_buf_t **out);

/** 
 *
 * @param[in] 
 *
 * @return 
 *
 * @see 
 */
int poll_recv_has_message (poll_recv_t pr);

/** 
 *
 * @param[in] 
 *
 * @return 
 *
 * @see 
 */
int poll_recv_is_alive (poll_recv_t pr);

/** 
 *
 * @param[in] 
 *
 * @return 
 *
 * @see 
 */
void poll_recv_del (poll_recv_t ps);

#endif // POLL_RECV_H

