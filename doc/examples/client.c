#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

#include <netdb.h>
#include <netinet/in.h>

#include <string.h>

#define h_addr h_addr_list[0]   /* for backward compatibility */
#define BUFFER_SIZE 256

int main(int argc, char const *argv[]) {
  int sockfd;
  int port_no;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  char buffer[BUFFER_SIZE];

  if (argc < 3) {
    fprintf(stderr,"Program %s needs arguments regarding target server: hostname, port\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  port_no = atoi(argv[2]);

  //Open socket
  sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sockfd < 0) {
    perror("Error: socket opening");
    exit(EXIT_FAILURE);
  }

  //Get server
  server = gethostbyname(argv[1]);	//maybe later use addrinfo
  if (server == NULL) {
    fprintf(stderr, "Error: No such host\n");
    exit(EXIT_FAILURE);
  }

  //Init server struct
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  memcpy(server->h_addr, &serv_addr.sin_addr.s_addr, server->h_length);  //why can't do assign direct? because of network order endian?
  serv_addr.sin_port = htons(port_no);  //convert to network order

  //Connect to server
  if ( connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))<0 ) {
    perror("Error: connection");
    exit(EXIT_FAILURE);
  }
  printf("Welcome to the chat !\n");
  printf("write '\\quit' in order to close this session \n\n");
  printf("Input msg:\n ");

  while  (1){
    printf(">>  ");
    memset(buffer, 0, BUFFER_SIZE);
    fgets(buffer, 20, stdin);

    //Write
    if (write(sockfd, buffer, 20) < 0) {		//maybe encompass it in while security loop, or use send...etc
      perror("Error: write");
      exit(EXIT_FAILURE);
    }
    printf("Msg sent: %s\n", buffer);

    //Read echo
    memset(buffer, 0, BUFFER_SIZE);
    if (read(sockfd, buffer, 20) < 0) {
      perror("Error: read echo");
      exit(EXIT_FAILURE);
    }
    printf("Echo received: %s", buffer);
  }
  printf("\n");

  return 0;
}
