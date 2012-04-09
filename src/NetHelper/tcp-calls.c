#include <errno.h>

#include "utils.h"
#include "sockaddr-helpers.h"

#include "tcp-calls.h"

int tcp_connect (const struct sockaddr *to)
{
    int fd;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        print_err("Connecting", "socket", errno);
        return -1;
    }

    if (connect(fd, to, sockaddr_size(to)) == -1) {
        print_err("Connecting", "connect", errno);
        close(fd);
        return -2;
    }

    return fd;
}

