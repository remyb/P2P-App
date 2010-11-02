/**
 temporary struct used to explain code
**/

struct llist {
  int number;
  char payload[1024];
  struct llist * next;
  struct llist * prev;
};

void printlru(struct llist * head);
struct llist * isCached(struct llist * head, int value);
void addNode(struct llist ** tail, struct llist * newNode);
void removeNode(struct llist ** node);
int getNodeCount(struct llist * head);
void popHead(struct llist ** head);
