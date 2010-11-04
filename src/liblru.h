/**
 temporary struct used to explain code
**/

#ifndef P2P_LRU_H
#define P2P_LRU_H
#include<stdio.h>
#include<stdlib.h>
#include "Cache.h"

//Remy work on data 
//Templates in C
#define decl_list(Type,data) typedef struct llist { \
  int number; \
  char payload[1024]; \
	Type data; \
  struct llist * next; \
  struct llist * prev;\
}List

decl_list(Cache,cache);


void printlru(struct llist * head);
struct llist * isCached(struct llist * head, Cache);
void addNode(struct llist ** tail, struct llist * newNode);
void removeNode(struct llist ** node);
int getNodeCount(struct llist * head);
void popHead(struct llist ** head);
void cleanAllNodes(struct llist ** head);

#endif
