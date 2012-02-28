/*
 *  Copyright (c) 2009 Luca Abeni
 *
 *  This is free software; see gpl-3.0.txt
 *
 *  This is a small test program for the gossip based TopologyManager
 *  To try the simple test: run it with
 *    ./topology_test -I <network interface> -P <port> [-i <remote IP> -p <remote port>]
 *    the "-i" and "-p" parameters can be used to add an initial neighbour
 *    (otherwise, the peer risks to stay out of the overlay).
 *    For example, run
 *      ./topology_test -I eth0 -P 6666
 *    on a computer, and then
 *      ./topology_test -I eth0 -P 2222 -i <ip_address> -p 6666
 *  on another one ... Of course, one of the two peers has to use -i... -p...
 *  (in general, to be part of the overlay a peer must either use
 *  "-i<known peer IP> -p<known peer port>" or be referenced by another peer).
 */
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>


#include "net_helper.h"
#include "peersampler.h"
#include "net_helpers.h"
#include "net_helper_all.h"

static struct psample_context *context;
static const char *my_addr = "127.0.0.1";
static int port = 6666;
static int mode = 1;
static int srv_port;
static const char *srv_ip;
static const char *buffer;
static char *fprefix;

static void cmdline_parse(int argc, char *argv[])
{
  int o;

  while ((o = getopt(argc, argv, "s:p:i:P:I:")) != -1) {
    switch(o) {
    case 's':
      mode = 0;
      break;
    case 'c':
      mode = 1;
    default:
      fprintf(stderr, "Error: unknown option %c\n", o);
      
      exit(-1);
    }
  }
}



static struct nodeID *init(void)
{
  struct nodeID *myID;

  myID = net_helper_init(my_addr, port, "");
  if (myID == NULL) {
    fprintf(stderr, "Error creating my socket (%s:%d)!\n", my_addr, port);

    return NULL;
  }
  //  context = psample_init(myID, NULL, 0, "protocol=cyclon");
  //context = psample_init(myID, NULL, 0, "");

  return myID;
}



 static void loop(struct nodeID *s)
{
  int *e = malloc(sizeof(int)*10);
  int fd, sfd;
  buffer = malloc(sizeof(char)*256);
  if (mode == 0) {
    
    sfd = tcp_serve(s, 0, e);
    fd = accept(fd,(struct sockaddr_in *) &s->data.addr, NULL);
    while(1) {
      read(fd, buffer);
      printf("%s\n",buffer);
    }
  }
  else {
    
    fd = tcp_connect(my_addr, port, &e);
    while(1) {
      send(fd, "hello world", 15, 1);
      sleep(1);
    }
    
  }
  
}


int main(int argc, char *argv[])
{
  struct nodeID *my_sock;

  cmdline_parse(argc, argv);

  my_sock = init();

  if (my_sock == NULL) {
    return -1;
  }

  loop(my_sock);

  return 0;
}
