#include "liblru.h"

  // Example Usage 
/*
void main() {
  struct llist * head = 0, * tail = 0, * temp = 0;

  temp = (struct llist *) malloc(sizeof(struct llist));
  temp->number = 9;

  printf("nodeCount: %d\n", getNodeCount(head));

  printlru(head);
  addNode(&head, temp);
  printlru(head);

  printf("nodeCount: %d\n", getNodeCount(head));

  temp = (struct llist *) malloc(sizeof(struct llist));
  temp->number = 100;
  addNode(&head, temp);
  printlru(head);

  temp = (struct llist *) malloc(sizeof(struct llist));
  temp->number = 20;
  addNode(&head, temp);
  printlru(head);
  
  temp = (struct llist *) malloc(sizeof(struct llist));
  temp->number = 20;
  cleanAllNodes(*head);
  printlru(head);
}
*/

/**
 Helper function for debugging
**/
void printlru(struct llist * head) {
  if(head == NULL) {
    printf("lru is empty\n");
    return;
  } else {
    struct llist * node;
    node = head;
    printf("h ");
    do {
      printf("[%s=>%s:%d] ",node->cache.name,inet_ntoa(node->cache.peer.ip),ntohs(node->cache.peer.port) );
      node = node->next;
    } while(node != NULL);
    printf("t \n");
  }
  return;
}

/**
 Determines if the value is cached; if so, return the node
**/
struct llist * isCached(struct llist * head, Cache cache) {
  if(head == NULL)
    return NULL;

  struct llist * node;
  node = head;

  do {
    //if(node->number == cache)
    if(cache.name == node->cache.name && cache.peer.ip == node->cache.peer.ip && cache.peer.port == node->cache.peer.port)
      return node;
    else
      node = node->next;
  } while(node != NULL);

  return NULL;
}

/**
 Adds the node to the linked list; requires the head and
 the new node being added. IF there are more than 4 nodes
 present we pop the least used one off. Tah dah = LRU!
**/
void addNode(struct llist ** head, struct llist * newNode) {
  struct llist * locator, * temp;

  if((*head) != NULL) {
    //locator = isCached((*head), newNode->number);
    //if(locator != NULL) {
    //  printf("found duplicate node!\n");
    //  removeNode(&locator);
    //}

    temp = (*head);
    if(temp->next != NULL) {
      do {
        temp = temp->next;
      } while(temp->next != NULL);
    }
    newNode->prev = temp;
    temp->next = newNode;

    if(getNodeCount((*head)) > 4)
      popHead(head);

  } else {
    (*head) = newNode;
  }
}

/**
 Removes any node no matter the position in the linked list.
 Pass the node to remove; use isCached function to determine node.
**/
void removeNode(struct llist ** node) {
  if((*node)->next != NULL && (*node)->prev != NULL) {
    (*node)->prev->next = (*node)->next;
    (*node)->next->prev = (*node)->prev;
  } else if((*node)->next != NULL && (*node)->prev == NULL) {
    (*node)->next->prev = NULL;
    (*node) = (*node)->next;
  } else if((*node)->next == NULL && (*node)->prev != NULL) {
    (*node)->prev->next = NULL;
    (*node) = (*node)->prev;
  }
}

void cleanAllNodes(struct llist **head) {
    while(*head != NULL){
        struct llist *temp = *head;
        free(*head);
        *head = temp->next;
    }
}

/**
 Pops the head of the linked list; requires the head of the linked list
**/
void popHead(struct llist ** head) {
  (*head)->next->prev = NULL;
  (*head) = (*head)->next;
}

/**
 Returns the count of the nodes in the linked list
**/
int getNodeCount(struct llist * head) {
  int counter = 0;

  if(head == NULL)
    return counter;
  else {
    do {
      counter++;
      head = head->next;
    } while(head != NULL);
  }
  return counter;
}
