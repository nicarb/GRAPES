/*
 *  Copyright (c) 2012 Arber Fama,
 *                     Giovanni Simoni
 *
 *  This is free software; see lgpl-2.1.txt
 */

#ifndef FAIR_H
#define FAIR_H

#include "dictionary.h"
#include "nh-types.h"

#include <sys/time.h>
#include <sys/select.h>

/** Fair select of the receiving peer.
 *
 * This function is a wrapper on the traditional select(2) system call.
 * Internally the `neighbours` dictionary will be scanned and its file
 * descriptors will be added to the `fdset` file-descriptors set. Then a
 * select(2) will be achieved and the resulting set will be scanned.
 *
 * The scan is performed in a fair order with respect to the neighbours,
 * and eventually the `conn` parameter will be setted if some neighbour
 * is willing to send some data.
 *
 * @param[in] neighbours The neighbours dictionary;
 * @param[in] timeout The timeout (or NULL for indefinte waiting);
 * @param[in] fdset A pre-initialized (and possibly non-empty) set of file
 *            descriptors to be analyzed;
 * @param[in] maxfd The maximum file descriptor in the set + 1 (or 0 if
 *            fdset is empty);
 * @param[out] conn The found connection (may be invalid: conn->fd == -1);
 * @param[out] e Pointer where errno will be stored in case of select(2)
 *               failure (you may pass NULL).
 *
 * @retval 2 If no neighbour is willing to send and the select(2) unlocked
 *           thanks to some other fd provided through the `fdset` paramter
 *           (the `conn` output parameter must be considered invalid);
 * @retval 1 if there's a node ready for reading (the `conn` parameter
 *           will contain references to the neighbor);
 * @retval 0 if there's not such a node (the `conn` output parameter must
 *           be considered invalid);
 * @retval -1 On select error.
 */
int fair_select(dict_t neighbours, struct timeval *timeout,
                fd_set *fdset, int maxfd, connection_t *conn, int *e);

#endif // FAIR_H

