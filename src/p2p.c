#include "p2p.h"

int main(int argc, char *argv[])
{
  if(argc == 1) {
    printf("usage: ./p2p port [peer socket] [filename]\n");
    exit(1);
  }

  int listeningPort = atoi(argv[1]);

  if(listeningPort == 0 || listeningPort < 1025 || listeningPort > 65536) {
    printf("error: use a port in the unreserved range\n");
    exit(1);
  } else {
    printf("Listening port -> %d/udp\n", listeningPort);
  }

  int countPeers = 0;
  int countFiles = 0;
  int loop = 0;

  struct p2p_peer * p2p_peers = (struct p2p_peer *) malloc(sizeof(struct p2p_peer));
  memset(p2p_peers, 0, sizeof(struct p2p_peer));

  struct p2p_peer * temp_peer = p2p_peers;
  int isFirstPeer = 1;

  struct p2p_file * p2p_files = (struct p2p_file *) malloc(sizeof(struct p2p_file));
  memset(p2p_files, 0, sizeof(struct p2p_file));

  struct p2p_file * temp_file = p2p_files;
  int isFirstFile = 1;
  
  for(loop = 2; loop < argc; loop++) {
    int index = containsChar(argv[loop], '/');
    if(index >= 0) {
      struct p2p_peer * p = createPeer(argv[loop]);
      if(p != NULL) {
	if(isPeerInList(p2p_peers, p) == 0) {
	  if(isFirstPeer == 1) {
	    p2p_peers = p;
	    temp_peer = p2p_peers;
	    isFirstPeer = 0;
	  } else {
	    temp_peer->next_peer = p;
	    temp_peer = temp_peer->next_peer;
	  }
	  countPeers++;
	}
      }
      continue;
    } else {
      struct p2p_file * f = createFile(argv[loop]);
      if(f != NULL) {
	if(isFileInList(p2p_files, f) == 0) {
	  if(isFirstFile == 1) {
            p2p_files = f;
            temp_file = p2p_files;
            isFirstFile = 0;
          } else {
            temp_file->next_file = f;
            temp_file = temp_file->next_file;
          }
	  countFiles++;
	}
      }
    }
  }

  if(countPeers > 0) {
    temp_peer = p2p_peers;
    char tempIp[INET_ADDRSTRLEN];
    int tempPort;
    do {
      tempPort = ntohs(temp_peer->socket.sin_port);
      inet_ntop(AF_INET, &(temp_peer->socket.sin_addr), tempIp, INET_ADDRSTRLEN);
      printf("Loaded peer -> %s:%d\n", tempIp, tempPort);
      if(temp_peer->next_peer != NULL)
	temp_peer = temp_peer->next_peer;
      else 
	temp_peer = NULL;
    } while(temp_peer != NULL);
  }

  if(countFiles > 0) {
    temp_file = p2p_files;
    do {
      printf("Loaded file -> %s\n", temp_file->filename);
      if(temp_file->next_file != NULL)
	temp_file = temp_file->next_file;
      else 
	temp_file = NULL;
    } while(temp_file != NULL);
  }

  /**
   Spawn threads
  **/
  //struct p2p_peer * p2p_peers - the list of p2p_peers in linked list
  //struct p2p_file * p2p_files - the list of p2p_files in linked list
  //using above data structures - perform initialization

  /* When p2p is first started, sends a listing message to all of its initial peers */

  /**
   Listen for stdin?
  **/
  // void listenOnStdin()

  /**
   Teardown of program
  **/
  temp_peer = p2p_peers;
  do {
    free(temp_peer);
    if(temp_peer->next_peer != NULL)
      temp_peer = temp_peer->next_peer;
    else 
      temp_peer = NULL;
  } while(temp_peer != NULL);

  temp_file = p2p_files;
  do {
    free(temp_file);
    if(temp_file->next_file != NULL)
      temp_file = temp_file->next_file;
    else
      temp_file = NULL;
  } while(temp_file != NULL);

  return 0;
}

struct p2p_peer * createPeer(char * argument) {
  int index = containsChar(argument, '/');
  char * ip   = getSubstring(argument, 0, index-1);
  char * port = getSubstring(argument, index+1, strlen(argument));

  struct p2p_peer * rv = (struct p2p_peer *) malloc(sizeof(struct p2p_peer));
  memset(rv, 0, sizeof(struct p2p_peer));

  if(atoi(port) < 1025 || atoi(port) > 65536) {
    printf("error: the argument %s must use a port in the unreserved range\n", argument);
    exit(1);
  } else {
    rv->socket.sin_port = htons(atoi(port));
  }

  if(inet_pton(AF_INET, ip, &(rv->socket.sin_addr)) != 1) {
    printf("error: the argument %s must use a valid ip address\n", argument);
    exit(1);
  }

  return rv;
}

/**
 Check to see if the peer is already added to the list
 Returns 0 if the peer_ip is not in the list
 Returns 1 if the peer_ip IS in the list
 Returns < 0 if an error has occurred
**/
int isPeerInList(struct p2p_peer * peers, struct p2p_peer * peer) {
  struct p2p_peer * temp = peers;

  if(temp == NULL && peer != NULL)
    return 0;

  if(peer == NULL)
    return -1; /* peer should NOT be NULL */

  if(temp != NULL && peer != NULL) {
    do {
      if(peer->socket.sin_addr.s_addr == temp->socket.sin_addr.s_addr)
        if(peer->socket.sin_port == temp->socket.sin_port)
          return 1;
      if(temp->next_peer != NULL)
        temp = temp->next_peer;
      else
        temp = NULL;
    } while(temp != NULL);
    return 0;
  }
  return -1; /* code should not be reached */
}

struct p2p_file * createFile(char * argument) {
  struct p2p_file * rv = (struct p2p_file *) malloc(sizeof(struct p2p_file));
  memset(rv, 0, sizeof(struct p2p_file));

  if(validFilename(argument) == 0) {
    printf("error: invalid filename %s\n", argument);
    exit(1);
  }

  FILE * fileio;
  fileio = fopen(argument, "r");
  if(fileio == NULL) {
    printf("error: missing a content file called %s\n", argument);
    exit(1);
  } else {
    fclose(fileio);
  }
  
  strcpy(rv->filename, argument);
  return rv;
}

/**
 Check to see if the peer is already added to the list
 Returns 0 if the peer_ip is not in the list
 Returns 1 if the peer_ip IS in the list
 Returns < 0 if an error has occurred
**/
int isFileInList(struct p2p_file * files, struct p2p_file * file) {
  struct p2p_file * temp = files;

  if(temp == NULL && file != NULL)
    return 0;

  if(file == NULL)
    return -1; /* file should NOT be NULL */

  if(temp != NULL && file != NULL) {
    do {
      if(strcmp(temp->filename,file->filename) == 0)
	return 1;
      if(temp->next_file != NULL)
	temp = temp->next_file;
      else 
	temp = NULL;
    } while(temp != NULL);
    return 0;
  }
  return -1; /* code should not be reached */
}

/**
 Pass in a peer and get the port as an int
**/
int getPeerPort(struct p2p_peer * peer) {
  if(peer == NULL)
    return -1;
  return ntohs(peer->socket.sin_port);
}

/**
 Pass in a peer and get the ip as a malloc'd char *
**/
char * getPeerIp(struct p2p_peer * peer) {
  if(peer == NULL)
    return NULL;

  char * rv = (char *) malloc(INET_ADDRSTRLEN);
  if(inet_ntop(AF_INET, &(peer->socket.sin_addr), rv, INET_ADDRSTRLEN) == NULL)
    return NULL;
  else
    return rv;
}

