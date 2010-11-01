#include "udp.h"

int ret_udp_sockfd(char *addr,char *port)
{
	int listening_fd;
	struct addrinfo hints;
	struct addrinfo *servinfo;
	int ret;
	memset(&hints,0,sizeof hints);
	hints.ai_family= AF_UNSPEC;//handle both Ipv4 and Ipv6
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
	if((ret = getaddrinfo(addr,port,&hints,&servinfo)) != 0)
	{
		perror("getaddrinfo");
		return -1;
	}

	struct addrinfo *p;
	for( p = servinfo; p!= NULL;p = p->ai_next)
	{
		printf("socktype = %d\n",p->ai_socktype);
			if((listening_fd = socket(p->ai_family,p->ai_socktype,p->ai_protocol)) == -1)
			{
				perror("socket");
				continue;
			}
			printf("%d\n",listening_fd);
			if(bind(listening_fd,p->ai_addr,p->ai_addrlen) == -1)
			{
				perror("bind");
				close(listening_fd);
				continue;
			}
			break;
	}
	if(p == NULL)
	{
		fprintf(stderr,"server: failed to bind\n");
		freeaddrinfo(servinfo);
		return -1;
	}

	freeaddrinfo(servinfo);
	return listening_fd;
}

void *handle_p2p_client(void *udphandler)
{
	UDPHandler *udp_handler = (UDPHandler *)udphandler;
	printf("listening fd = %d\n",udp_handler->sockfd);
	printf("holla");

}
#ifdef DEBUG
int main()
{
	UDPHandler udphandler;
	udphandler.ret_listening_sockfd = ret_udp_sockfd;
	udphandler.sockfd = udphandler.ret_listening_sockfd("localhost","3490");
	//ret_udp_sockfd(NULL,"4950");
	pthread_t tids[3];
	pthread_create(&tids[0],NULL,handle_p2p_client,(void *)&udphandler);




  pthread_join(tids[0],NULL);
	return 0;
}


#endif
