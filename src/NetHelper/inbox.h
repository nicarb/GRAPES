#ifndef INBOX_H
#define INBOX_H

#include "dictionary.h"
#include "client.h"

<<<<<<< HEAD
/* 
   This module provides relative datastructure for the inbox management.
 */

typedef struct inbox * inbox_t;

/** 
 *
 * @param[in] 
 *
 * @return 
 *
 * @see 
 */
inbox_t inbox_new ();

/** 
 *
 * @param[in] 
 *
 * @return 
 *
 * @see 
 */
void inbox_del (inbox_t ib);

/** 
 *
 * @param[in] 
 *
 * @return 
 *
 * @see 
 */
int inbox_empty (inbox_t ib);

/** 
 *
 * @param[in] 
 *
 * @return 
 *
 * @see 
 */
void inbox_scan_dict (inbox_t ib, dict_t dict);

/** 
 *
 * @param[in] 
 *
 * @return 
 *
 * @see 
 */
=======
typedef struct inbox * inbox_t;

inbox_t inbox_new ();

void inbox_del (inbox_t ib);

int inbox_empty (inbox_t ib);

void inbox_scan_dict (inbox_t ib, dict_t dict);

>>>>>>> 032b4dc85bcc84143b81cc44170867d424eab1a7
client_t inbox_next (inbox_t ib);

#endif // INBOX_H

