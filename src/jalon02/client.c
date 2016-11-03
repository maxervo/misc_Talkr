#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h> // struct sockaddr_in

#include "common.h"
#include "client.h"


int main(int argc, char const *argv[]) {
  int sockfd;
  int port_no;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  char buffer[BUFFER_SIZE];

  printf("-------------------------\n- Talkr Client ! -\n-------------------------\n");
  printf("Write '/quit' in order to close this session\n");

  //Verifying arguments
  if (argc < 3) {
    fprintf(stderr,"Program %s needs arguments regarding target server: hostname, port\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  port_no = atoi(argv[2]);

  //Preparing
  sockfd = create_socket();
  server = get_server(argv[1]);
  init_serv_address(server, &serv_addr, port_no);

  //Connect to server
  if ( connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))<0 ) {
    error("Error - connection");
  }

  //Receive Welcome/Refuse msg
  memset(buffer, 0, BUFFER_SIZE);
  do_recv(sockfd, buffer, BUFFER_SIZE);
  printf("%s\n", buffer);
  if (strcmp(buffer, REFUSE_MSG) == 0){
    error("Error - connection");
  }

  //Main Client Loop
  while(1) {  //maybe do more elegant way
    printf("Input msg: ");
    memset(buffer, 0, BUFFER_SIZE);
    fgets(buffer, BUFFER_SIZE, stdin);

    do_send(sockfd, buffer, BUFFER_SIZE);
    printf("Msg sent\n");

    //Quit
    if (strcmp(buffer, QUIT_MSG) == 0) {
      printf("Client decided to quit the chat\n");
      close(sockfd);
      return EXIT_SUCCESS;
    }

    //Receive Echo
    memset(buffer, 0, BUFFER_SIZE);
    do_recv(sockfd, buffer, BUFFER_SIZE);
    printf("Msg received: %s\n", buffer);

  }

  return EXIT_SUCCESS;  //optional line, indeed client quits the program with "/quit" or ctrl+c
}


struct hostent* get_server(const char *host_target) {
  struct hostent *server = gethostbyname(host_target);	//Later on: use addrinfo (cf. gethostbyname considered deprecated, and for ipv6...etc)
  if (server == NULL) {
    fprintf(stderr, "Error: No such host\n");
    exit(EXIT_FAILURE);
  }

  return server;
}

void init_serv_address(struct hostent* server, struct sockaddr_in* serv_addr_ptr, int port_no) {
  memset(serv_addr_ptr, 0, sizeof(struct sockaddr_in));
  serv_addr_ptr->sin_family = AF_INET;
  memcpy(server->h_addr, &(serv_addr_ptr->sin_addr.s_addr), server->h_length);
  serv_addr_ptr->sin_port = htons(port_no);  //convert to network order
}
