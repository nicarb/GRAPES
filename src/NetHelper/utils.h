#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <stdint.h>

#include "nh-types.h"

<<<<<<< HEAD
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
 * @see 
 */
void print_err (const char *where, const char *what, int e);

/** Continue sending until it's completed, return 0 or -1 on success or failure
 * respectively
 *
 * @param[in] fd
 * @param[in] buffer
 * @param[in] buflen
 *
 * @return 
 *
 * @see 
 */
int send_forced (int fd, const uint8_t * buffer, size_t buflen);

/** Cycle on receive until it's completed, return 0 or -1  on success or failure,
 * respectively.
 * @param[in] fd
 * @param[in] buffer
 * @param[in] bufflen
 *
 * @return 
 *
 * @see 
 */
int recv_forced (int fd, uint8_t * buffer, size_t buflen);

/** 
 *
 * @param[in] hdr
 *
 * @return the size of the header
 *
 * @see 
 */
ssize_t header_get_size (header_t *hdr);

/** Sets the size of the headder
 *
 * @param[in] hdr headder structure
 * @param[in] s the size of the headder
 *
 *
 * @see 
 */
void header_set_size (header_t *hdr, size_t s);

=======
/* like calling malloc, just abort()s if return null */
void * mem_new (size_t size);

/* like calling realloc, just abort()s if return null */
void * mem_renew (void * ptr, size_t size);

/* allocate a copy of the given memory area */
void * mem_dup (const void * src, size_t size);

/* print error on standard error */
void print_err (const char *where, const char *what, int e);

/* send until completed, return 0 or -1 on success and failure
 * respectively */
int send_forced (int fd, const uint8_t * buffer, size_t buflen);

/* receive until completed, return 0 or -1 on success and failure
 * respectively */
int recv_forced (int fd, uint8_t * buffer, size_t buflen);

ssize_t header_get_size (header_t *hdr);

void header_set_size (header_t *hdr, size_t s);

int invalid_fd (int fd);

>>>>>>> 032b4dc85bcc84143b81cc44170867d424eab1a7
#endif // UTILS_H

