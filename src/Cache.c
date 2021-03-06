#include "Cache.h"
#include "liblru.h"

//remember to protect these vars across Threads
List *head_data_cache = NULL;
List *head_content_cache = NULL;


int write_cache_to_file(Cache *cache)
{
	
	FILE *fd = fopen(cache->name,"a+");
	fprintf(fd,"%s",inet_ntoa(cache->peer.ip));
	fprintf(fd,":");
	char buff[8];
	snprintf(buff,8,"%d",ntohs(cache->peer.port));
	fprintf(fd,"%s",buff);
	fprintf(fd,":");
	fprintf(fd,"\n");
	fclose(fd);
}
int is_content_present_in_dir(const char *content,Cache *cache)
{
}


#ifdef DEBUG
int main()
{
	Cache cache;
	cache.peer.ip = 100;
	cache.peer.port = 10;
	strcpy(cache.name,"edo");
	write_cache_to_file(&cache);

	return 0;
}

#endif
