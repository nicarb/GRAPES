#ifndef TIMEOUT_H
#define TIMEOUT_H

#include <sys/time.h>

typedef struct tout * tout_t;

/** 
 *
 * @param[in] 
 *
 * @return 
 *
 * @see 
 */
tout_t tout_new (const struct timeval *timeout); 

/** 
 *
 * @param[in] 
 *
 * @return 
 *
 * @see 
 */
tout_t tout_copy (const tout_t t); 

/** 
 *
 * @param[in] 
 *
 * @return 
 *
 * @see 
 */
void tout_reset (tout_t t);

/** 
 *
 * @param[in] 
 *
 * @return 
 *
 * @see 
 */
int tout_expired (tout_t t);

/** 
 *
 * @param[in] 
 *
 * @return 
 *
 * @see 
 */
void tout_del (tout_t t);

/** 
 *
 * @param[in] 
 *
 * @return 
 *
 * @see 
 */
unsigned tout_timeval_to_ms (const struct timeval *tval);

/** 
 *
 * @param[in] 
 *
 * @return 
 *
 * @see 
 */
unsigned tout_now_ms ();

#endif // TIMEOUT_H

