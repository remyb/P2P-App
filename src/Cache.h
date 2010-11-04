#ifndef P2P_CACHE_H
#define P2P_CACHE_H
#include<stdio.h>
#include<stdint.h>

//LOCAL CONTENT 
//given to p2p as command line arguments




typedef struct _Peer
{
	unsigned int ip; //}  ==> Pair 
	uint16_t port;   //}
}Peer;
//DATA Cache
//cache upto 5 contents

//CONTENT CACHE
//16 names and Peer addresses
//(A,x),(B,x)  ==> (peer,name), peer ==> ip:port (4+2 ) = 6 bytes

typedef struct _Cache
{
	char name[20]; //filename
	Peer peer;

}Cache;
//Cache data_cache,content_cache;

int write_cache_to_file(Cache *cache);
int is_content_present_in_dir(const char *content,Cache *cache);


#endif
