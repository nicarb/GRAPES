#ifndef TIMEOUT_H
#define TIMEOUT_H

#include <sys/time.h>

typedef struct tout * tout_t;

/** Initializes a new structure for the timeout
 *
 * @param[in] timeout is the value of the time till expires
 *
 * @return the tout_t structure created
 *
 * @see 
 */
tout_t tout_new (const struct timeval *timeout); 

/** Copy the timeout structure
 *
 * @param[in] t source data structure
 *
 * @return a new tout_t structure corresponding to a copy of t
 *
 * @see 
 */
tout_t tout_copy (const tout_t t); 

/** The structure is resetted each time a new event reguarding such
 * node.
 *
 * @param[in] t is the source t
 *
 * @see 
 */
void tout_reset (tout_t t);

/** Resets the structure each time a timeout expires. This means that
 * the values inside the data structure are not valid anymore
 * but the structure is not deallocated.
 *
 * @param[in] t the target data structure
 *
 * @return 0 or -1 in cse of success or failure, respectively
 *
 * @see 
 */
int tout_expired (tout_t t);

/** Deletes all the data structure.
 *
 * @param[in] t the target data structure
 *
 *
 * @see 
 */
void tout_del (tout_t t);

/** 
 *
 * @param[in] tval time value
 *
 * @return m_seconds is the time value expressed in m-secs.
 *
 * @see 
 */
unsigned tout_timeval_to_ms (const struct timeval *tval);

/** Returns 
 *
 *
 * @return a positive value corresponding to the timeout
 *
 * @see 
 */
unsigned tout_now_ms ();

#endif // TIMEOUT_H

