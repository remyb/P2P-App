#include "udp.h"
#include <fcntl.h>
#include "messageheader.h"


#define nonblock(sock) (fcntl(sock,F_SETFL,O_NONBLOCK))
#define LOCK(thread) pthread_mutex_lock(thread)
#define UNLOCK(thread) pthread_mutex_unlock(thread)
//global Vars
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

//pthread callback
//handles UDP connections
void *handle_p2p_client(void *udphandler)
{
	pthread_detach(pthread_self());
	thread_increment();
	UDPHandler *udp_handler = (UDPHandler *)udphandler;
	struct sockaddr_in src_addr;
	socklen_t tsz;
	GenericMsg genmsg;
	//write your sending/recv message logic
	while(1)
	{
		tsz=sizeof src_addr;
		bzero(&genmsg,sizeof genmsg);
		//handle messages 
//TODO
/*
Receiving a Data Message

When p2p receives a data message corresponding to an outstanding request, it must save the data in a file (on disk, in the current directory) with the given name. It must also keep a copy of the contents in its data cache. p2p must also add the originator/name pair to the content directory if they are not already present, and must in any case update the use time for the entry. The originator is obtained from the recvfrom call. Finally, the request must be deleted from the list of outstanding requests.

If a data message is received that does not correspond to any outstanding requests (or for which the request was sent more than 5 seconds ago), the data message should be discarded without modifying the caches and without saving to a file. p2p may print a message reporting this event.
Receiving a Request Message

If p2p receives a request and does not have the content and cannot locate the content in its content directory, the request must be discarded.

If p2p receives a request message and has the specified content, either in the local content or in the data cache, p2p will in response send the corresponding data message. This data message will carry this p2p's address and port number in the originator field. If the content was in the data cache, the entry is updated to the current time for the LRU algorithm, i.e. this entry becomes the newest entry.

If p2p does not have the content, but the content directory lists one or more peers who do have the content, the p2p will return a try message to the originator, listing all the peers in its content directory that are listed as having the content. This is equivalent to an iterative DNS query. A p2p getting such a request should also initiate its own request for this content, as follows.
Sending a Request Message

When there is a request for content, either requested by the user or by another peer, p2p must generate a request.

If there are more than 3 outstanding requests (each request may be to multiple peers), the oldest outstanding request is removed, and the new one is added instead.

If the requested content is listed in the content directory, a request is sent to each of the peers listed as having the content. This counts as a single outstanding request.

Otherwise, and only if this is a user request, the request is sent once to each of the peers listed in the content directory, even though they do not list the requested content. Again, this counts as a single outstanding request.
Receiving a Try Message

When p2p receives a try message corresponding to a user request, it must add all of the listed peers to its content directory. The process then resumes as described under "sending a Request Message", except that, optionally, p2p may only send the request to any new peers that were not previously known to have the content. If there are no new peers for this content, p2p need not send any request messages.

Each item should be requested at most 5 times, that is, there can be at most 5 sequences of request messages sent for each user request.

An item being requested as a result of receiving a request message, as described in the last sentence of "Receiving a Request Message", should not be requested more than once. That is, receiving a try message in response to such a request should result in updating the content directory but not sending further requests.
Receiving a Listing Message

When p2p receives a listing message, it adds the information to its content directory. The peer's address is obtained from the IP number and port number returned by the recvfrom system call. 
*/
   
		if(recvfrom(udp_handler->sockfd,&genmsg,sizeof genmsg,0,(struct sockaddr *)&src_addr,&tsz) <= 0)
		{
			perror("recvfrom");
			thread_decrement();
			pthread_exit(NULL);
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
			List *dataNode = (List *)malloc(sizeof(List));
			strcpy(dataNode->cache.name,genmsg.datamsg.content_name);
			printf("datanode peer name= %s\n",dataNode->cache.name);
			//ips/ports
			dataNode->cache.peer.ip = src_addr.sin_addr.s_addr;
			dataNode->cache.peer.port = src_addr.sin_port;
			printf("Ip = %s, Port = %d\n",inet_ntoa(dataNode->cache.peer.ip),ntohs(dataNode->cache.peer.port));
			dataNode->cache.content_len = genmsg.datamsg.content_len;
			dataNode->cache.content = (char *) malloc(dataNode->cache.content_len);
			strncpy(dataNode->cache.content,genmsg.datamsg.content,dataNode->cache.content_len);
			printf("Content = %s\n",dataNode->cache.content);
			//use mutex here
			if(isCached(head_data_cache,dataNode->cache) == NULL)
				addNode(&head_data_cache,dataNode);
			printlru(head_data_cache);
			
			//TODO
			//do free at the end
			//free(dataNode->cache.content);
			//free(dataNode);
			break;
			case 0xCC:
			fprintf(stderr,"GOT CONTROL MESSAGE");
			break;
			default:
			fprintf(stderr,"wrong magic_no = %d",genmsg.magic_no);
		}
		//fprintf(stderr,"client says %s\n",buff);
	}
	thread_decrement();

	pthread_exit(NULL);

}
//handle stdin 
void *handle_stdin(void *iohandler)
{
	pthread_detach(pthread_self());
	thread_increment();
	int age;
	//scanf("%d",&age);
	//printf("Your age is %d",age);
	thread_decrement();
	pthread_exit(NULL);
}


//sending listing messages
/*
Sending Listing Messages

p2p sends a listing message every 240 seconds (every 4 minutes) to every peer listed in its content directory or on its command line. Only one message is sent to each peer, even if the peer appears multiple times in the content directory and on the command line.

The contents of the message are the local contents, the contents of the data cache, and the contents of the content directory. If the total number of items to be sent exceeds the maximum length of the listing message (56 entries), only send as many entries as fit in the message. You may choose, any way you wish, which entries to send. 

			break;
			case 0xCC:
			break;
			default:
			fprintf(stderr,"wrong magic_no = %d",magic_no);
		}
		//fprintf(stderr,"client says %s\n",buff);
	}
	thread_decrement();

	pthread_exit(NULL);

}
*/

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
	UDPHandler udphandler;
	udphandler.ret_listening_sockfd = ret_udp_sockfd;
	socklen_t len;
	struct sockaddr_in remote_addr;
	while(1)
	{
		sleep(240); //4 mins
		udphandler.sockfd = udphandler.ret_listening_sockfd("0.0.0.0","0");
		int peer_no = 0;
		ListingMsg lmsg;
		lmsg.control_msg.magic_no = 0xCC;
		lmsg.control_msg.selector_value = 0x4C; //#define MACRO ???
		lmsg.message_count = 10; //change it later;
		int message_no = 0;
		socklen_t len = sizeof remote_addr;
		for(;message_no < lmsg.message_count;++message_no)
		{
			//populate Resource Name here.
			//populate Remote Address here.
			sendto(udphandler.sockfd,&lmsg,sizeof lmsg,0,(struct sockaddr *)&remote_addr,len);

		}	
		
		
		
	    /*
	    use this struct
        typedef struct _ListingMsg
        {
          ControlMsgSign control_msg;
          //uint8_t magic_no; //0xCC
          //uint8_t selector_value;//0x4C
          uint16_t message_count;
          ResourceName res_name[56]; //56 * 26
        }packed_data ListingMsg
        defined in messageheader.h
        
        ListingMsg lst_msg;
        bzero(&lst_msg,sizeof lst_msg);
        lst_msg.control_msg.magic_no = 0xCC;
         ............ . selector_value = 0x4C;
        other variables as well
        then sendto(  &lst_msg,.... );
        ResourceName === Content/ dir lists
        */
		//TODO
		//read from ./content/ dir
		//calculate no. of peers
		//fill remote_addr
		//remote_addr.sin_port /IP stufs 
//loop starts
		
		//sendto(udphandler.sockfd,&msg,sizeof msg,(struct sockaddr *)&remote_addr,len);
		//loop ends
		close(udphandler.sockfd);
		
	}
}
#define MAX_THREADS 3
int main(int argc,char **argv)
{
	UDPHandler udphandler;
	IOHandler iohandler;
	TOHandler timeouthandler;
	udphandler.ret_listening_sockfd = ret_udp_sockfd;
	udphandler.sockfd = udphandler.ret_listening_sockfd("localhost","3490");
	//ret_udp_sockfd(NULL,"4950");
	pthread_t tids[MAX_THREADS];
	pthread_create(&tids[0],NULL,handle_p2p_client,(void *)&udphandler);
	//thread from sending listing message
	//TODO
	//later, take the argument from command line and pass it as argument
	//argc,argv 
	pthread_create(&tids[1],NULL,send_listing_msg,NULL);
	pthread_create(&tids[2],NULL,handle_timeouts,(void *)&timeouthandler);
	//	}





	pthread_join(tids[0],NULL);
	pthread_join(tids[1],NULL);
	pthread_join(tids[2],NULL);
	return 0;
}

