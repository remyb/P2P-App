#ifndef P2P_UDP_H
#define P2P_UDP_H
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<unistd.h>
#include <pthread.h>
#include "Cache.h"
#include "liblru.h"

extern List *head_data_cache;
extern List *head_content_cache;

typedef struct _UDPHandler
{
	int sockfd;
	int (*ret_listening_sockfd)(char *addr,char *port);
}UDPHandler;
int ret_udp_sockfd(char *addr,char *port);

typedef struct _IOHandler
{
	int l;
}IOHandler;
typedef struct _TOHandler
{
	int l;
}TOHandler;
//pthread callbacks
void *handle_p2p_client(void *udphandler);
void *handle_stdin(void *iohandler);
void *handle_timeouts(void *timeouthandler);
void *send_listing_msg(void *);


extern Cache data_cache;
extern Cache content_cache;

#endif
