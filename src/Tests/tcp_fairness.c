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

static const int MAX_RETRY = 5;
static const size_t N_SENDERS = 10;

#define NPEERS    5

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
      //ret->buffers[j] = calloc(strlen(TEST_STRING) + 2, 
      //                         sizeof(char));
      j ++;
    }
  }

  ret->turn_receiver = NULL;

  return ret;
}

static void cleanup (peer_data_t *d)
{
  free(d);
}
  
static void do_stats (peer_data_t *pdata, struct nodeID *self, uint32_t *stats)
{
  int retry = MAX_RETRY;
  
  uint32_t the_id;

  while (retry) {
    struct timeval tout = {0, 500000};
    
    if (wait4data(self, &tout, NULL) == 1) {
      struct nodeID *remote;
      ssize_t N;

      fprintf(stderr, "Woken up. Now going in recv\n");
      
      N = recv_from_peer(self, &remote, (void *) &the_id, sizeof(uint32_t));
      
      pdata->counter[the_id] += N;
              
      fprintf(stderr, "Received %i bytes from %s [tot=%d, retry=%i]\n",
              (int)N, node_addr(remote), pdata->counter[the_id], retry);
      
      if (N <= 0) {
        retry = 0;
      }
      nodeid_free(remote);
    } else {
              retry --;
              fprintf(stderr, "Timeout, retry=%i\n", retry);
    }
  }
  

  fprintf(stderr, "End of experiment (retry was %i)\n", retry);
}

static int sender_sync (peer_data_t *data)
{
  struct nodeID *remote;
  uint32_t N;
  uint32_t my_id = data->id;
  
  remote = data->peers[my_id];
  
  fprintf(stderr, "Sender waiting for receiver sync\n");
  N = recv_from_peer(data->self, &remote, (void *) &my_id, sizeof(uint32_t));

  if (N == -1) {
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
  printf("----------------- Stats ---------------------------------\n");
  printf("cumulative receive %u.\n",
         (unsigned)data->counter[my_id]);
  printf("---------------------------------------------------------\n");
  
  return 0;
}

static peer_data_t *run_peer (int id)
{
  peer_data_t *data = init(id);

  struct nodeID *self;
  struct nodeID *receiver;
  int N, retry=MAX_RETRY;

  self = data->self;
  receiver = data->peers[id];
  
  N = send_to_peer(self, receiver,(void *) id, sizeof(uint32_t));

  if (N < 0) {
    fprintf(stderr, "Sender %i send anomalous, N=%i\n", id, (int) N);

    retry --;
    if (retry == 0) {
      fprintf(stderr, "Sender %i giving up.\n", id);
    }
  }
  else {
    data->counter[id] += N;
  }
  
  return data;
}

int main (int argc, char **argv)
{
  pid_t peers[N_SENDERS + 1];
  int i;
  int status;
  peer_data_t* peerdata[NPEERS];
  
  for (i = 0; i <= NPEERS; i ++) {
    pid_t P = fork();
    
    if (P == 0) {
      peerdata[i] = run_peer(i);
      
      exit(EXIT_SUCCESS);
    }
            peers[i] = P;
  }

  if ((peers[0] = fork()) == 0) {
    // some statistics about the peers
    do_stats(peerdata[0], peerdata[0]->self, &status);
    
    exit(EXIT_SUCCESS);
  }
  
  for (i = 1; i <= N_SENDERS; i ++) {
    do_stats(peerdata[i], peerdata[i]->self, &status);
    sender_sync(peerdata[i]);

    //do_stats(peerdata[i], peerdata[i]->self, &status);
    cleanup(peerdata[i]);
    waitpid(peers[i], &status, 0);
    printf("Peer %i exited with status %i\n", i,
                   WEXITSTATUS(status));
  }
  
  exit(EXIT_SUCCESS);
}
