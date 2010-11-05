#include "udp.h"
#include <fcntl.h>
#include "messageheader.h"
#include "filehandler.h"

#define MAX_THREADS 3

#define nonblock(sock) (fcntl(sock,F_SETFL,O_NONBLOCK))
#define LOCK(thread) pthread_mutex_lock(thread)
#define UNLOCK(thread) pthread_mutex_unlock(thread)


//global Vars
UDPHandler udphandler;
char *p2p_port;
int thread_count = 0;
pthread_mutex_t threadwatch_1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t threadwatch_2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t threadwatch_3 = PTHREAD_MUTEX_INITIALIZER;
//remove it later
pthread_mutex_t threadwatch = PTHREAD_MUTEX_INITIALIZER;


void thread_increment()
{
	LOCK(&threadwatch);
	++thread_count;
	UNLOCK(&threadwatch);
}

void thread_decrement()
{
	LOCK(&threadwatch);
	--thread_count;
	UNLOCK(&threadwatch);
}

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
			//nonblock(listening_fd);
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

void add_to_data_content(const struct sockaddr_in *src_addr, const GenericMsg *genmsg)
{
	Cache cache;
	strcpy(cache.name,genmsg->datamsg.content_name);
	cache.peer.ip= src_addr->sin_addr.s_addr;
	cache.peer.port= src_addr->sin_port;

	//dataNode->cache.peer.ip = src_addr->sin_addr.s_addr;
	///dataNode->cache.peer.port = src_addr->sin_port;
	//printf("Ip = %s, Port = %d\n",inet_ntoa(dataNode->cache.peer.ip),ntohs(dataNode->cache.peer.port));
	//dataNode->cache.content_len = genmsg->datamsg.content_len;
	//dataNode->cache.content = (char *) malloc(dataNode->cache.content_len);
	//strncpy(dataNode->cache.content,genmsg->datamsg.content,dataNode->cache.content_len);
	//use mutex here

	if(isCached(head_data_cache,cache,0) == NULL)
	{
		List *dataNode = (List *)malloc(sizeof(List));
		strcpy(dataNode->cache.name,genmsg->datamsg.content_name);
		printf("datanode peer name= %s\n",dataNode->cache.name);
		addNode(&head_data_cache,dataNode);
		printlru(head_data_cache);
	}
	if(isCached(head_content_cache,cache,1) == NULL)
	{
		List *content_cache_node = (List *)malloc(sizeof(List));
		strcpy(content_cache_node->cache.name,genmsg->datamsg.content_name);
		content_cache_node->cache.peer.ip = src_addr->sin_addr.s_addr;
		content_cache_node->cache.peer.port = src_addr->sin_port;
		addNode(&head_content_cache,content_cache_node);
		printlru(head_content_cache);
	}
	//if file is not present in content dir, dump the content 
	//char *path = (char *)malloc(1 + strlen(genmsg->datamsg.content_name));
	//strcat(path,genmsg->datamsg.content_name);
	if(!file_exist(genmsg->datamsg.content_name))
		create_content(genmsg->datamsg.content_name,genmsg->datamsg.content,genmsg->datamsg.content_len);
}

void on_recv_req_msg(GenericMsg *genmsg)
{
	Cache cache;
	List *node = NULL;
	if(!file_exist(genmsg->reqmsg.name) && isCached(head_content_cache,cache,1) == NULL /*does not have content in content dir*/)
	{
		//if(can not locate the content) then discard
		fprintf(stderr,"Discarding message for %s, Absent in content/\n",genmsg->reqmsg.name);
		return;
	}
	else if((node = isCached(head_data_cache,cache,0)) == NULL) 
		{
			//send try message 
			struct sockaddr_in remote_addr;
			bzero(&remote_addr,sizeof remote_addr);
			GenericMsg genmsg_try;
			bzero(&genmsg_try,sizeof genmsg_try);
			genmsg_try.trymsg.control_msg.magic_no = 0xCC;
			genmsg_try.trymsg.control_msg.selector_value = 0x54;
			strcpy(genmsg_try.trymsg.name,genmsg->reqmsg.name);

			if(sendto(udphandler.sockfd,&genmsg_try,sizeof genmsg_try,0,(struct sockaddr *)&remote_addr,sizeof remote_addr))
			{
				perror("sendto");

			}
		}
	else if((node = isCached(head_content_cache,cache,1)) != NULL)
	{
		
		GenericMsg genmsg_data;
		struct sockaddr_in remote_addr;
		bzero(&remote_addr,sizeof remote_addr);
		bzero(&genmsg_data,sizeof genmsg_data);
		//UDPHandler udphandler;
		//bzero(&udphandler,sizeof udphandler);
		//udphandler.ret_listening_sockfd  = ret_udp_sockfd;
		//udphandler.sockfd = udphandler.ret_listening_sockfd("0.0.0.0","0");
		//populate genmsg_data
		genmsg_data.datamsg.magic_no = 0xDD;
		strcpy(genmsg_data.datamsg.content_name,genmsg->reqmsg.name);
		remote_addr.sin_family = AF_INET;
		remote_addr.sin_addr.s_addr = node->cache.peer.ip;
		remote_addr.sin_port = node->cache.peer.port;
		printf("%s:%d\n",inet_ntoa(remote_addr.sin_addr),ntohs(remote_addr.sin_port));
		readWholeFile(genmsg->reqmsg.name,&genmsg_data.datamsg.content_len);

		if(sendto(udphandler.sockfd,&genmsg_data,sizeof genmsg_data,0,(struct sockaddr *)&remote_addr,sizeof remote_addr))
		{
			perror("sendto");
		
		}
	}

}
void on_recv_try_msg(GenericMsg *genmsg)
{
  Cache cache;
  // 1) parse the message for the peers
  // 2) add the peers in the content directory
  // 3) for each received peer, send a request message to NEW peers only
  // 4) max messages request sent are 5 

}

void on_recv_listing_msg(GenericMsg *genmsg)
{
  // 1) when receiving a listing message, add the information to the content directory
  // 2) obtain ip and port from recvfrom call
}

//pthread callback
//handles UDP connections
void * handle_p2p_client(void *data)
{
	pthread_detach(pthread_self());
	thread_increment();
	struct sockaddr_in src_addr;
	socklen_t tsz;
	GenericMsg genmsg;
	//write your sending/recv message logic
	while(1)
	{
		tsz=sizeof src_addr;
		bzero(&genmsg,sizeof genmsg);
		//handle messages 

		if(recvfrom(udphandler.sockfd,&genmsg,sizeof genmsg,0,(struct sockaddr *)&src_addr,&tsz) <= 0)
		{
			perror("recvfrom");
			thread_decrement();
			//pthread_exit(NULL);
		}
		//DEBUG
		print_ntwkbytes(genmsg.ntwbytes,sizeof genmsg);
		fprintf(stderr,"magic no = 0x%X\n",genmsg.magic_no);

		switch(genmsg.magic_no)
		{
			//DATA message
			case 0xDD:
				//process genmsg.datamsg 
				fprintf(stderr,"GOT DATA MESSAGE\n");
				add_to_data_content(&src_addr,&genmsg);

				break;
			case 0xCC:
				fprintf(stderr,"GOT CONTROL MESSAGE");
				switch(genmsg.ctrlmsgsign.selector_value)
				{
					//Req Msg
					case 0x52:
						fprintf(stderr,"GOT Req Message\n");
						on_recv_req_msg(&genmsg);
						break;
						//Try Msg
					case 0x54:
						fprintf(stderr,"GOT Try Message\n");
						on_recv_try_msg(&genmsg);
						break;
						//Listing Message
					case 0x4C:
						fprintf(stderr,"GOT Listing Message\n");
						on_recv_listing_msg(&genmsg);
						break;
					default:
						fprintf(stderr,"Invalid selector value\n");

				}
				break;
			default:
				fprintf(stderr,"wrong magic_no = %d",genmsg.magic_no);
		}
	}
	thread_decrement();

	pthread_exit(NULL);

}
//handle stdin 
void *handle_stdin(void *iohandler)
{
	pthread_detach(pthread_self());
	thread_increment();

	char input[40];
	printf("Type !quit <enter> to quit p2p\n");
	while(1) {
	  memset(&input, 0, 40);
	  getInput(input, 40);

	  if(strncmp(input,"!quit",5) == 0) {
	    printf("Quitting...\n");
			{
				exit(1);
			}
	    break;
	  } else {
	    if(validFilename(input) == 0) {
	      printf("Error: Invalid filename %s\n", input);
	    } else {
	      if(isFilenameInList(input) == 1) {
		printf("Success: Requested content '%s' FOUND in the LOCAL cache!\n", input);
	      } else if(0) { // check content cache

	      } else if(0) { // check data cache

	      } else if(0) { // perform "send a request message"

	      } else {
		printf("Failure: Requested content '%s' NOT found in any cache or remote peer.\n", input);
	      }
	    }
	  }
	}


	thread_decrement();
	pthread_exit(NULL);
}


//handle timeouts
void *handle_timeouts(void *timeouthandler)
{
	pthread_detach(pthread_self());
	thread_increment();
	//wait for 5 secs
	while(1)
	{
		sleep(5);
		//write your timeout stuffs here
    //TODO
/*
Timeouts
Any outstanding request that is older than 5 seconds should be deleted. At this time, if the content is listed in the content directory, all content directory entries for this content should be removed, since the request failed and we cannot trust that the peers have the content they are listed as having. 
*/
		//say hi to Ami
		fprintf(stderr,"Hi Ami\n");
	}
	thread_decrement();
	pthread_exit(NULL);
}

//sending listing messages
/*
Sending Listing Messages

p2p sends a listing message every 240 seconds (every 4 minutes) to every peer listed in its content directory or on its command line. Only one message is sent to each peer, even if the peer appears multiple times in the content directory and on the command line.

The contents of the message are the local contents, the contents of the data cache, and the contents of the content directory. If the total number of items to be sent exceeds the maximum length of the listing message (56 entries), only send as many entries as fit in the message. You may choose, any way you wish, which entries to send. 
*/
void *send_listing_msg(void *data)
{
	//UDPHandler udphandler;
	//udphandler.ret_listening_sockfd = ret_udp_sockfd;
	socklen_t len;
	struct sockaddr_in remote_addr;
	while(1)
	{
		sleep(10); //4 mins //TODO
		GenericMsg genmsg;
		bzero(&genmsg,sizeof genmsg);
		genmsg.lstmsg.control_msg.magic_no = 0xCC;
		genmsg.lstmsg.control_msg.selector_value = 0x4C; //#define MACRO ???
		genmsg.lstmsg.message_count = getNodeCount(head_content_cache); //change it later;
		socklen_t len = sizeof remote_addr;
		if(head_content_cache==NULL)
		{
			fprintf(stderr,"content lru is empty\n");
			continue;
		}
		else
		{
			List *node = head_content_cache;
			do
			{
				bzero(&remote_addr,sizeof remote_addr);
				remote_addr.sin_family = AF_INET;
				remote_addr.sin_addr.s_addr = node->cache.peer.ip;
				remote_addr.sin_port = node->cache.peer.port;
				//printf("%s:%d\n",inet_ntoa(remote_addr.sin_addr),ntohs(remote_addr.sin_port));

				//populate Resource Name here.
				//populate Remote Address here.
				if(sendto(udphandler.sockfd,&genmsg,sizeof genmsg,0,(struct sockaddr *)&remote_addr,len) <= 0)
				{
					perror("sendto");
					fprintf(stderr,"Failed to send data to %s:%d\n",inet_ntoa(remote_addr.sin_addr),ntohs(remote_addr.sin_port));
				}
#ifdef DEBUG
				print_ntwkbytes(genmsg.ntwbytes,sizeof genmsg);
#endif

				node = node->next;
			}while(node != NULL);	
		}
	}

	}

#ifdef DEBUG
	int main(int argc,char **argv)
	{
		if(argc != 2)
		{
			fprintf(stderr," %s <port>\n",argv[0]);
			exit(-1);
		}
		p2p_port = argv[1];
		printf("started listening at port = %s\n",p2p_port);

		IOHandler iohandler;
		TOHandler timeouthandler;
		udphandler.ret_listening_sockfd = ret_udp_sockfd;
		udphandler.sockfd = udphandler.ret_listening_sockfd("0.0.0.0",p2p_port);
		pthread_t tids[MAX_THREADS];
		pthread_create(&tids[0],NULL,handle_p2p_client,(void *)&udphandler);
		//TODO
		//later, take the argument from command line and pass it as argument
		//argc,argv 
		pthread_create(&tids[1],NULL,send_listing_msg,(void *)&udphandler);
		pthread_create(&tids[2],NULL,handle_timeouts,(void *)&timeouthandler);


		pthread_join(tids[0],NULL);
		pthread_join(tids[1],NULL);
		pthread_join(tids[2],NULL);
		close(udphandler.sockfd);
		return 0;
}

#endif
