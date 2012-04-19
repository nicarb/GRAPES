#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <stdint.h>

#include "nh-types.h"

/** Similar to calling malloc, just abort()s if returns null
 *
 * @param[in] size
 *
 *
 * @see malloc
 */
void * mem_new (size_t size);

/** Similar to calling realloc, just abort()s if returns null 
 *
 * @param[in] 
 *
 * @return 
 *
 * @see 
 */
void * mem_renew (void * ptr, size_t size);


/** Allocates a copy of the given memory area 
 *
 * @param[in] src 
 * @param[in] size
 * @return 
 *
 * @see 
 */
void * mem_dup (const void * src, size_t size);


/** Print error on standard error 
 *
 * @param[in] where
 * @param[in] what
 * @param[in] e
 *
 * @return 
 *
 * @see 
 */
void print_err (const char *where, const char *what, int e);

/* send until completed, return 0 or -1 on success and failure
 * respectively */
/** 
 *
 * @param[in] 
 *
 * @return 
 *
 * @see 
 */
int send_forced (int fd, const uint8_t * buffer, size_t buflen);

/* receive until completed, return 0 or -1 on success and failure
 * respectively */
/** 
 *
 * @param[in] 
 *
 * @return 
 *
 * @see 
 */
int recv_forced (int fd, uint8_t * buffer, size_t buflen);

/** 
 *
 * @param[in] 
 *
 * @return 
 *
 * @see 
 */
ssize_t header_get_size (header_t *hdr);

/** 
 *
 * @param[in] 
 *
 * @return 
 *
 * @see 
 */
void header_set_size (header_t *hdr, size_t s);

#endif // UTILS_H

