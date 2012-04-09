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

typedef struct dict * dict_t;
typedef struct dict_data * dict_data_t;

dict_t dict_new (struct tag *);

void dict_del (dict_t);

dict_data_t dict_search (dict_t, const struct sockaddr *);

int * dict_data_fd (dict_data_t);

uint8_t * dict_data_buffer (dict_data_t, size_t);

void dict_data_update (dict_data_t);

#endif // DICTIONARY_H

