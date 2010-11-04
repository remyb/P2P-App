#include "udp.h"
#include <fcntl.h>
#include "messageheader.h"
#include "filehandler.h"


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

void add_to_data_content(const struct sockaddr_in *src_addr,const GenericMsg *genmsg)
{
			List *dataNode = (List *)malloc(sizeof(List));
			strcpy(dataNode->cache.name,genmsg->datamsg.content_name);
			printf("datanode peer name= %s\n",dataNode->cache.name);
			dataNode->cache.peer.ip = src_addr->sin_addr.s_addr;
			dataNode->cache.peer.port = src_addr->sin_port;
			printf("Ip = %s, Port = %d\n",inet_ntoa(dataNode->cache.peer.ip),ntohs(dataNode->cache.peer.port));
			dataNode->cache.content_len = genmsg->datamsg.content_len;
			dataNode->cache.content = (char *) malloc(dataNode->cache.content_len);
			strncpy(dataNode->cache.content,genmsg->datamsg.content,dataNode->cache.content_len);
			printf("Content = %s\n",dataNode->cache.content);
			//use mutex here
			if(isCached(head_data_cache,dataNode->cache) == NULL)
				addNode(&head_data_cache,dataNode);
			printlru(head_data_cache);
			write_cache_to_file(&dataNode->cache);
}

void on_recv_req_msg(GenericMsg *genmsg,Cache cache)
{
	if(!file_exist(genmsg->reqmsg.name))
	{
		fprintf(stderr,"Discarding message for %s, Absent in content/\n",genmsg->reqmsg.name);
		return;
	}
	else
	{
		//search the content in that file
	}
	//TODO search into local content
	//Currently only doing for data cache
	List *node_found = NULL;
	if((node_found = isCached(head_data_cache,cache)) != NULL)
	{
		if(strcmp(node_found->name,genmsg.reqmsg.name) == 0)
		{
			//send data message
		}
	}


	

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
			//ips/ports
			add_to_data_content(&src_addr,&genmsg);
			
			//TODO
			//do free at the end
			//free(dataNode->cache.content);
			//free(dataNode);
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
				on_recv_try_msg();
				break;
				//Listing Message
				case 0x4C:
				fprintf(stderr,"GOT Listing Message\n");
				on_recv_listing_msg();
				break;
				default:

			}

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

