#include "messageheader.h"

void print_ntwkbytes(char *buff,int len)
{
	int i;
	for(i = 0; i < len; ++i) {
		printf("%X",(uint8_t)buff[i]);
		printf(" ");
	}
	printf("\n");
}
