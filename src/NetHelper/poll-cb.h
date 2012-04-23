#ifndef POLL_CB_H
#define POLL_CB_H

#include <stdint.h>

<<<<<<< HEAD
/* 
   
 */

=======
>>>>>>> 032b4dc85bcc84143b81cc44170867d424eab1a7
typedef struct pollcb * pollcb_t;

typedef void * (* pollcb_cb_t) (void *ctx, int fd, int epollfd);

<<<<<<< HEAD

/** 
 *
 * NOTE: fd gets duplicated
 *
 * @param[in] cb 
 * @param[in] ctx 
 * @param[in] fd
 * @param[in] epollfd 
 *
 * @return  a pollcb structure ...
 *
 * @see 
 */
pollcb_t pollcb_new (pollcb_cb_t cb, void * ctx, int fd, int epollfd);

/** Delete the pollcb structure
 *
 * @param[in] p 
 *
 *
 * @see 
 */
void pollcb_del (pollcb_t p);

/** Run pollcb by assigning to p's context the corresponding p->cb(...)
 *
 * @param[in] p
 *
 *
 * @see 
 */
void pollcb_run (pollcb_t p);

/** 
 *
 * @param[in] p
 * @param[in] events
 *
 * @return 
 *
 * @see 
 */
int pollcb_enable (pollcb_t p, uint32_t events);

/** Disable pollcb referred to p
 *
 * @param[in] p
 *
 * @return 
 *
 * @see 
 */
int pollcb_disable (pollcb_t p);

/** Check if the desidered fd is still alive or not.
 *
 * @param[in] p
 *
 * @return 
 *
 * @see 
 */
=======
/* NOTE: fd gets duplicated */
pollcb_t pollcb_new (pollcb_cb_t cb, void * ctx, int fd, int epollfd);

void pollcb_del (pollcb_t p);

void pollcb_run (pollcb_t p);

int pollcb_enable (pollcb_t p, uint32_t events);

int pollcb_disable (pollcb_t p);

>>>>>>> 032b4dc85bcc84143b81cc44170867d424eab1a7
int pollcb_is_alive (pollcb_t p);

#endif // POLL_CB_H
