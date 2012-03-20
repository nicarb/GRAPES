#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/types.h>
#include <assert.h>
#include <string.h>

#include "net_helper.h"

static const uint16_t START_PORT = 9400;
static const char TEST_STRING[] =
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec a diam"
    "lectus. Sed sit amet ipsum mauris. Maecenas congue ligula ac quam"
    "viverra nec consectetur ante hendrerit. Donec et mollis dolor."
    "Praesent et diam eget libero egestas mattis sit amet vitae augue. Nam"
    "tincidunt congue enim, ut porta lorem lacinia consectetur. Donec ut"
    "libero sed arcu vehicula ultricies a non tortor. Lorem ipsum dolor sit"
    "amet, consectetur adipiscing elit. Aenean ut gravida lorem. Ut turpis"
    "felis, pulvinar a semper sed, adipiscing id dolor. Pellentesque auctor"
    "nisi id magna consequat sagittis. Curabitur dapibus enim sit amet elit"
    "pharetra tincidunt feugiat nisl imperdiet. Ut convallis libero in urna"
    "ultrices accumsan. Donec sed odio eros. Donec viverra mi quis quam"
    "pulvinar at malesuada arcu rhoncus. Cum sociis natoque penatibus et"
    "magnis dis parturient montes, nascetur ridiculus mus. In rutrum"
    "accumsan ultricies.  Mauris vitae nisi at sem facilisis semper ac in"
    "est.";

static const size_t READ_BUFSIZE = 10;
static const int MAX_RETRY = 5;
static const size_t N_SENDERS = 10;

<<<<<<< HEAD
typedef struct {
    int id;
    struct nodeID * peers[NPEERS];
    struct nodeID * self;
    struct nodeID * turn_receiver;
    char *buffers[NPEERS];
    int counter[NPEERS];
} peer_data_t;

static peer_data_t * init (int id)
{
    int i, j;
    peer_data_t * ret = malloc(sizeof(peer_data_t));
    assert(ret != NULL);

    ret->id = id;

    for (j = i = 0; i <= NPEERS; i ++) {
        uint16_t port = START_PORT + i;

        ret->counter[i] = 0;
        if (i == id) {
            fprintf(stderr, "Server for node, id=%i, port=%hu\n", id, port);
            ret->self = net_helper_init("127.0.0.1", port, NULL);
        } else {
            fprintf(stderr, "Target connection for node %i at port %hu\n",
                    id, port);
            ret->peers[j] = create_node("127.0.0.1", port);
            
            // strlen(TEST_STRING) + 2: 1 def + 1 as preamble for identifying the sender
            ret->buffers[j] = calloc(strlen(TEST_STRING) + 2, 
                                     sizeof(char));
            j ++;
        }
    }

    ret->turn_receiver = NULL;

    return ret;
}

static void cleanup (peer_data_t *d)
=======
void run_receiver ()
>>>>>>> 736143199a6a5487771c0d3028f3283b11291540
{
    struct nodeID *self;
    int retry;
    size_t cumulate;

<<<<<<< HEAD

static void do_stats (peer_data_t *pdata, struct nodeID *self, uint32_t *stats)
{
    int retry = MAX_RETRY;

    uint32_t the_id;

=======
    self = net_helper_init("127.0.0.1", START_PORT, "tcp_backlog=10");
    fprintf(stderr, "Receiver ready\n");

    retry = MAX_RETRY;
    cumulate = 0;
>>>>>>> 736143199a6a5487771c0d3028f3283b11291540
    while (retry) {
        struct timeval tout = {0, 500000};

        if (wait4data(self, &tout, NULL) == 1) {
            struct nodeID *remote;
            uint8_t buffer[READ_BUFSIZE];
            ssize_t N;

<<<<<<< HEAD
            fprintf(stderr, "Woken up. Now going in recv\n");

            N = recv_from_peer(self, &remote, (void *) &the_id, sizeof(uint32_t));

            pdata->counter[the_id] += N;

            fprintf(stderr, "Received %i bytes from %s [tot=%d, retry=%i]\n",
                    (int)N, node_addr(remote), pdata->counter[the_id], retry);
            
            if (N <= 0) {
                retry = 0;
=======
            N = recv_from_peer(self, &remote, buffer, READ_BUFSIZE);
            if (N < 0) {
                retry --;
                fprintf(stderr, "Recv failed, retry=%i\n", retry);
            } else {
                fprintf(stderr, "Received %i bytes\n", (int)N);
                cumulate += N;
                retry = MAX_RETRY;
>>>>>>> 736143199a6a5487771c0d3028f3283b11291540
            }
            nodeid_free(remote);
        } else {
            retry --;
            fprintf(stderr, "Timeout, retry=%i\n", retry);
        }
    }

<<<<<<< HEAD
    fprintf(stderr, "End of experiment (retry was %i)\n", retry);
}

#if 0
static void send_lorem_ipsum (struct nodeID *self, struct nodeID *target)
{
    int running = 1;

    const char *text = TEST_STRING;
    ssize_t len = strlen(TEST_STRING) + 1;

    fprintf(stderr, "Sender starting to send...\n");
    while (running) {
        size_t N;

        switch (N = send_to_peer(self, target, text, len)) {
            case -1:
                fprintf(stderr, "Error in sending\n");
            case 0:
                running = 0;
                break;
            default:
                text += N;
                len -= N;
                if (len <= 0) {
                    running = 0;
                }
                break;
        }
    }
}
#endif

static int sender_sync (peer_data_t *data)
{
    struct nodeID *remote;

    uint32_t my_id = data->id;

    fprintf(stderr, "Sender waiting for receiver sync\n");
    if (recv_from_peer(data->self, &remote, (void *) &my_id, sizeof(uint32_t)) == -1) {
        return -1;
    }

    if (data->turn_receiver != NULL) {
        nodeid_free(data->turn_receiver);
    }
    data->turn_receiver = remote;

    fprintf(stderr, "Sender completing sync with %s\n", node_addr(remote));
    if (send_to_peer(data->self, remote, (void *) &my_id, sizeof(uint32_t)) == -1) {
        return -1;
    }
    return 0;
=======
    printf("----------------- Stats ---------------------------------\n");
    printf("cumulative receive: %u (approx %u × %u = %u)\n",
           (unsigned)cumulate,
           (unsigned)N_SENDERS,
           (unsigned)strlen(TEST_STRING),
           (unsigned)(N_SENDERS * strlen(TEST_STRING)));
    printf("---------------------------------------------------------\n");

    nodeid_free(self);
>>>>>>> 736143199a6a5487771c0d3028f3283b11291540
}

void run_sender (int i)
{
<<<<<<< HEAD
    int i;
    
    struct nodeID *remote;
    uint32_t my_id = data->id;
=======
    struct nodeID *self;
    struct nodeID *receiver;
    const uint8_t *buffer;
    ssize_t buflen;
    int retry;
>>>>>>> 736143199a6a5487771c0d3028f3283b11291540

    self = net_helper_init("127.0.0.1", START_PORT + i, "tcp_backlog=1");
    receiver = create_node("127.0.0.1", START_PORT);

<<<<<<< HEAD
        fprintf(stderr, "Receiver, sync with %s\n",
                node_addr(data->peers[i]));
        if (send_to_peer(data->self, data->peers[i], (void *) &my_id,
                         sizeof(uint32_t)) == -1) {
            return -1;
        }
    }

    while (i > 0) {
        fprintf(stderr, "Sync, %i missing...\n", i);
        if (recv_from_peer(data->self, &remote, (void *) &my_id,
                           sizeof(uint32_t)) == -1) {
            return -1;
        }
        i --;
    }
    fprintf(stderr, "Receiver sync completed\n");
=======
    fprintf(stderr, "Sender %i waiting 0.75 sec...\n", i);
    usleep(750000);

    fprintf(stderr, "Sender %i starting transmission.\n", i);
>>>>>>> 736143199a6a5487771c0d3028f3283b11291540

    buffer = TEST_STRING;
    buflen = strlen(TEST_STRING);

<<<<<<< HEAD
static peer_data_t *run_peer (int id)
{
    peer_data_t *data = init(id);
    int turn;
=======
    retry = MAX_RETRY;
    while (buflen > 0 && retry) {
        ssize_t N;
>>>>>>> 736143199a6a5487771c0d3028f3283b11291540

        N = send_to_peer(self, receiver, buffer, READ_BUFSIZE);
        if (N < 0) {
            fprintf(stderr, "Sender %i send anomalous, N=%i\n", i, (int)N);
            retry --;
            if (retry == 0) {
                fprintf(stderr, "Sender %i giving up.\n", i);
            }
        } else {
            buffer += N;
            buflen -= N;
        }
    }
<<<<<<< HEAD
    return data;
    
=======

    nodeid_free(receiver);
    nodeid_free(self);
>>>>>>> 736143199a6a5487771c0d3028f3283b11291540
}

int main (int argc, char **argv)
{
    pid_t peers[N_SENDERS + 1];
    int i;

<<<<<<< HEAD
    peer_data_t* peerdata[NPEERS];

    for (i = 0; i <= NPEERS; i ++) {
        pid_t P = fork();

        if (P == 0) {
            peerdata[i] = run_peer(i);

=======
    for (i = 1; i <= N_SENDERS; i ++) {
        pid_t P = fork();

        if (P == 0) {
            run_sender(i);
>>>>>>> 736143199a6a5487771c0d3028f3283b11291540
            exit(EXIT_SUCCESS);
        }
        peers[i] = P;
    }
    if ((peers[0] = fork()) == 0) {
        run_receiver();
        exit(EXIT_SUCCESS);
    }

    for (i = 0; i <= N_SENDERS; i ++) {
        int status;
        
        // some statistics about the peers
        do_stats(peerdata[i], peerdata[i]->self, &status);
        cleanup(peerdata[i]);
        waitpid(peers[i], &status, 0);
        printf("Peer %i exited with status %i\n", i,
               WEXITSTATUS(status));
    }

    exit(EXIT_SUCCESS);
}

