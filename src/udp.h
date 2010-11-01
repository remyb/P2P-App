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

typedef struct _UDPHandler
{
	int sockfd;
	int (*ret_listening_sockfd)(char *addr,char *port);
}UDPHandler;

int ret_udp_sockfd(char *addr,char *port);
//pthread callback
void *handle_p2p_client(void *udphandler);



#endif
