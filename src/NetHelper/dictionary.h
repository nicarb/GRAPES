/*
 *  Copyright (c) 2012 Arber Fama,
 *                     Giovanni Simoni
 *
 *  This is free software; see lgpl-2.1.txt
 */

#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <netinet/in.h>
#include "config.h"
#include "sockaddr-helpers.h"

typedef struct dict * dict_t;
typedef struct dict_data * dict_data_t;

typedef void (* dict_delcb_t) (void * user);
typedef int (* dict_pred_t) (void * user);

/* Return 1 to continue, 0 to stop scanning */
typedef int (* dict_foreach_t) (void *ctx, const sockaddr_t *,
                                void * userdata, uint32_t *flags);

/* Destructor must support NULL as input value */
dict_t dict_new (struct tag *, dict_delcb_t del, dict_pred_t valid);

void dict_del (dict_t);

dict_data_t dict_search (dict_t, const sockaddr_t *);

void dict_foreach (dict_t, dict_foreach_t cb, void * ctx);

int * dict_data_fd (dict_data_t);

void ** dict_data_user (dict_data_t);

#endif // DICTIONARY_H

