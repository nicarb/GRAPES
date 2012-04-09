#include "timeout.h"
#include "utils.h"

#include <sys/time.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

struct tout {
    struct timeval last_update;
    struct timeval period;
};

tout_t tout_new (const struct timeval *timeout)
{
    tout_t ret;

    ret = mem_new(sizeof(struct tout));

    memcpy((void *)&ret->period, (const void *)timeout,
           sizeof(struct timeval));
    tout_update(ret);

    return ret;
}

tout_t tout_copy (const tout_t t)
{
    tout_t ret;

    ret = mem_dup(t, sizeof(struct tout));
    tout_update(ret);

    return ret;
}

void tout_reset (tout_t t)
{
    gettimeofday(&t->last_update, NULL);
}

int tout_expired (tout_t t)
{
    struct timeval now;
    struct timeval diff;

    gettimeofday(&now, NULL);
    timersub(&now, &t->last_update, &diff);

    return timercmp(&diff, &t->period, >);
}

void tout_del (tout_t t)
{
    free(t);
}

