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
    printf("listening port: %d/udp\n", listeningPort);
  }

  int countPeers = 0;
  int countFiles = 0;
  int loop = 0;

  for(loop = 2; loop < argc; loop++) {
    if(containsChar(argv[loop], '/') >= 0)
      countPeers++;
    else
      countFiles++;
  }

  #ifdef DEBUG
    printf("Peer Count: %d\n", countPeers);
    printf("File Count: %d\n", countFiles);
  #endif

  /**
   Build an array of p2p peers using sockaddr_in
  **/
  struct sockaddr_in ** p2p_peers = NULL;
  p2p_peers = malloc(countPeers * sizeof(struct sockaddr_in *));

  int temp = 2;
  for(loop = 0; loop < countPeers; loop++) {
    p2p_peers[loop] = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in));

    for(; temp < argc; temp++) {
      int index = containsChar(argv[temp], '/');
      if(index >= 0) {
	char * peer = getSubstring(argv[temp], 0, index-1);
	char * port = getSubstring(argv[temp], index+1, strlen(argv[temp]));

	if(atoi(port) < 1025 || atoi(port) > 65536) {
	  printf("error: the argument %s must use a port in the unreserved range\n", argv[temp]);
	  exit(1);
	}
	(p2p_peers[loop])->sin_port = htons(atoi(port));

	if(inet_pton(AF_INET, peer, &(p2p_peers[loop])->sin_addr) != 1) {
	  printf("error: the argument %s must use a valid ip address\n", argv[temp]);
	  exit(1);
	}

	free(peer);
	free(port);

	temp++;
	break;
      }
    }
  }

  /**
   Verify p2p files are valid
  **/
  for(temp = 2; temp < argc; temp++) {
    if(containsChar(argv[temp], '/') == -1) {

      if(validFilename(argv[temp]) == 0) {
        printf("error: invalid filename %s\n", argv[temp]);
        exit(1);
      }

      FILE * fileio;
      fileio = fopen(argv[temp], "r");
      if(fileio == NULL) {
	printf("error: missing a content file called %s\n", argv[temp]);
	exit(1);
      } else {
	fclose(fileio);
      }
    }
  }

  /**
   Listen for incoming requests
  **/
  // startListener(listeningPort); // untested code

  /**
   Teardown of program
  **/
  for(loop = 0; loop < countPeers; loop++) {
    free(p2p_peers[loop]);
  }
  free(p2p_peers);

  return 0;
}

void startListener(int udpPort) {
  int socket_fd;
  //int client_fd; // needed?
  int status = 0;
  //  int numOfBytes = 0; // needed?

  char udpService[10];
  //char udpBuffer[256]; // needed?

  struct addrinfo hints;
  struct addrinfo *listener;
  struct sockaddr_storage client_addr;
  socklen_t addr_len;

  sprintf(udpService, "%d", udpPort);
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;

  if((status = getaddrinfo(NULL, udpService, &hints, &listener)) != 0) {
    fprintf(stderr, "error: getaddrinfo -> %s\n", gai_strerror(status));
    exit(1);
  }

  socket_fd = socket(listener->ai_family, listener->ai_socktype,
                      listener->ai_protocol);
  if(socket_fd == -1) {
    fprintf(stderr, "error: socket -> %s\n", strerror(errno));
    exit(1);
  }

  if((bind(socket_fd, listener->ai_addr, listener->ai_addrlen)) == -1) {
    fprintf(stderr, "error: bind -> %s\n", strerror(errno));
    exit(2);
  }

  printf("listener: waiting to recvfrom...\n");

  addr_len = sizeof(client_addr);

  //numOfBytes = recvfrom(socket_fd, udpBuffer, 

  //*/
}
