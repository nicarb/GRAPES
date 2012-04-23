#ifndef SOCKADDR_HELPERS_H
#define SOCKADDR_HELPERS_H

#include <netinet/in.h>
#include <stdint.h>

<<<<<<< HEAD
/** 
 *
 * @param[in] sockaddr the target needed
 *
 * @return the size of the target sockaddr
 *
 * @see 
 */
size_t sockaddr_size (const struct sockaddr *);

/** Function for creating a an hash-key for the
 *  given sockaddr.
 *
 * @param[in] sockaddr
 *
 * @return hashing key obtained by the sockaddr.
 *         This key is needed in the Dictionary.
 *
 * @see 
 */
uintptr_t sockaddr_hash (const struct sockaddr *);

/** Comparision function
 *
 * @param[in] source1
 * @param[in] source2
 *
 * @return 
 *
 * @see 
 */
int sockaddr_cmp (const struct sockaddr *sa0,
                  const struct sockaddr *sa1);

/** Checks if two sockaddresses are the same
 *
 * @param[in] source1
 * @param[in] source2
 *
 * @return 
 *
 * @see 
 */
int sockaddr_equal (const struct sockaddr *sa0,
                    const struct sockaddr *sa1);

/** 
 *
 * @param[in] dst
 * @param[in] defsize
 * @param[in] src
 *
 * @return 
 *
 * @see 
 */
int sockaddr_dump (void *dst, size_t dstsize, const struct sockaddr *src);

/** 
 *
 * @param[in] dst
 * @param[in] dstsize
 * @param[in] src
 *
 * @return 
 *
 * @see 
 */
int sockaddr_undump (struct sockaddr *dst, size_t dstsize,
                     const void *src);

/** Copies the data of the src sockaddr to the dst one.
 *
 * @param[in] dst
 * @param[in] src
 *
 * @return a copy of the src sockaddr data
 *
 * @see 
 */
struct sockaddr * sockaddr_copy (struct sockaddr *dst, 
                                 const struct sockaddr * src);

/** Duplicate a sockaddr structure
 *
 * @param[in] src
 *
 * @return the duplicated structure
 *
 * @see 
 */
struct sockaddr * sockaddr_dup (const struct sockaddr *src);

/** 
 *
 * @param[in] sockaddr
 * @param[in] buffer
 * @param[in] buflen
 *
 * @return 
 *
 * @see 
 */
int sockaddr_strrep (const struct sockaddr *, char *buffer, size_t buflen);

/** 
 *
 * @param[in] ouraddr
 * @param[in] fd
 *
 * @return 
 *
 * @see 
 */
int sockaddr_send_hello (const struct sockaddr *ouraddr, int fd);

/** 
 *
 * @param[in] theiraddr
 * @param[in] fd
 *
 * @return 
 *
 * @see 
 */
int sockaddr_recv_hello (struct sockaddr *theiraddr, int fd);

/** Specific calls for sockaddr_in 
 *
 * @param[in] in
 * @param[in] ipaddr
 * @param[in] port
 *
 * @return 
 *
 * @see 
 */
int sockaddr_in_init (struct sockaddr_in *in, const char *ipaddr,
=======
typedef union {
    struct sockaddr sa;
    struct sockaddr_storage ss;
    struct sockaddr_in sin;
    struct sockaddr_in6 sin6;
} sockaddr_t;

size_t sockaddr_size (const sockaddr_t *);

uintptr_t sockaddr_hash (const sockaddr_t *);

int sockaddr_cmp (const sockaddr_t *sa0, const sockaddr_t *sa1);

int sockaddr_equal (const sockaddr_t *sa0, const sockaddr_t *sa1);

int sockaddr_dump (void *dst, size_t dstsize, const sockaddr_t *src);

int sockaddr_undump (sockaddr_t *dst, size_t dstsize, const void *src);

sockaddr_t * sockaddr_copy (sockaddr_t *dst, const sockaddr_t * src);

sockaddr_t * sockaddr_dup (const sockaddr_t *src);

const char * sockaddr_strrep (const sockaddr_t *, char *buffer,
                              size_t buflen);

uint16_t sockaddr_getport (const sockaddr_t *);

int sockaddr_send_hello (const sockaddr_t *ouraddr, int fd);

int sockaddr_recv_hello (sockaddr_t *theiraddr, int fd);

/* Specific calls for sockaddr_in */

int sockaddr_in_init (sockaddr_t *in, const char *ipaddr,
>>>>>>> 032b4dc85bcc84143b81cc44170867d424eab1a7
                      uint16_t port);

#endif // SOCKADDR_HELPERS_H

