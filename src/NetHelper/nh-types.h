/*
 *  Copyright (c) 2012 Arber Fama,
 *                     Giovanni Simoni
 *
 *  This is free software; see lgpl-2.1.txt
 */

#ifndef NH_TYPES_H
#define NH_TYPES_H

#include <arpa/inet.h>
#include <stdint.h>

typedef struct {
    uint32_t size;
} header_t;

typedef struct {
    int fd;
    const struct sockaddr *addr;
} connection_t;

#endif // NH_TYPES_H

