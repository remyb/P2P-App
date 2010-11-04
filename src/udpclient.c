#include "udp.h"
#include <fcntl.h>
#include "messageheader.h"

//This program does the  testing only
int ret_udp_sockfd(char *addr,char *port)
{
	struct addrinfo *p;
	char buff[] = "Holla ami";
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

	for( p = servinfo; p!= NULL;p = p->ai_next)
	{
			if((listening_fd = socket(p->ai_family,p->ai_socktype,p->ai_protocol)) == -1)
			{
				perror("socket");
				continue;
			}
			printf("%d\n",listening_fd);
			break;
	}
	if(p == NULL)
	{
		fprintf(stderr,"server: failed to bind\n");
		freeaddrinfo(servinfo);
		return -1;
	}
	GenericMsg genmsg;
	bzero(&genmsg,sizeof genmsg);
	genmsg.magic_no = 0xDD;
	strcpy(genmsg.datamsg.content_name,"edo");
	genmsg.datamsg.content_len = 3;
	strcpy(genmsg.datamsg.content,"He");

	if(sendto(listening_fd,&genmsg,sizeof genmsg,0,p->ai_addr,p->ai_addrlen) <= -1)
	{
		perror("sendto");
		exit(1);
	}

	freeaddrinfo(servinfo);
	return listening_fd;
}

//pthread callback
int main()
{
	UDPHandler udphandler;
	udphandler.ret_listening_sockfd = ret_udp_sockfd;
	udphandler.sockfd = udphandler.ret_listening_sockfd("localhost","3490");
	printf("listening _ fd = %d\n",udphandler.sockfd);

	close(udphandler.sockfd);

	return 0;
}

