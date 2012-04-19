/*
 *  Copyright (c) 2012 Arber Fama,
 *                     Giovanni Simoni
 *
 *  This is free software; see lgpl-2.1.txt
 */

#ifndef NH_TYPES_H
#define NH_TYPES_H

#include <arpa/inet.h>

/* 
   Sockaddr and relative fd are put in a general purpose data structure for the connection management.
 */

typedef struct {
    int fd;
    const struct sockaddr *addr;
} connection_t;

#endif // NH_TYPES_H

