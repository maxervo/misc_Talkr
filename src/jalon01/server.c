#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

//Later on: add a common .h
#define MAX_NO_CONNECTIONS 20
#define MAX_NUM_QUEUE 10
#define BUFFER_SIZE 256
#define QUIT_MSG "/quit\n"  //Attention newline character captured as well by input

#define KEEP_COMMUNICATION 1
#define CLOSE_COMMUNICATION 0

void error(const char *msg);
int handle(int sockfd);
int do_socket();
void init_serv_address(struct sockaddr_in *serv_addr_ptr, int port_no);
void do_bind(int sockfd, struct sockaddr_in *serv_addr_ptr);

int main(int argc, char* argv[]) {
  struct sockaddr_in serv_addr, cli_addr;
  int sockfd, new_sockfd;
  int cli_len = sizeof(cli_addr);
  int port_no;

  if (argc != 2) {
      fprintf(stderr, "Usage: RE216_SERVER port\n");
      return EXIT_FAILURE;
  }
  port_no = atoi(argv[1]);

  //Preparing
  sockfd = do_socket();
  init_serv_address(&serv_addr, port_no);
  do_bind(sockfd, &serv_addr);

  //Listen
  listen(sockfd, MAX_NUM_QUEUE);

  //Server Loop : Acceptance Queue
  int i;
  for (i=0; i < MAX_NO_CONNECTIONS; i++) {
    new_sockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &cli_len);   //Blocked, (Sockets config: Blocking), so only one connection opened at a time
    if (new_sockfd < 0) {
      error("Error - accept");
    }
    while(handle(new_sockfd)){     //while the connection is open
      //Handling each time
    }
    close(new_sockfd);
  }
  close(sockfd);

  return EXIT_SUCCESS;
}

void error(const char *msg)   //ATTENTION : program flow exit
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int handle(int sockfd) {
  char buffer[BUFFER_SIZE];
  memset(buffer, 0, BUFFER_SIZE);
  int readlen = recv(sockfd, buffer, BUFFER_SIZE, 0);     //Later on: add a security "do while" loop for bytes, interesting for busy interface or embedded systems with small network buffer

  //Quit
  if (readlen == 0 || strcmp(buffer,QUIT_MSG) == 0) { //other than quit msg, recv returns 0 when client closes connection as well
    printf("Connection closed by client\n");
    return CLOSE_COMMUNICATION;
  }

  //Error
  else if (readlen < 0) {
    error("Error - reception");
  }

  //Msg
  else {
    printf("msg received : %s", buffer);

    //Echo
    if (send(sockfd, buffer, BUFFER_SIZE, 0) < 0) {       //Later on: add a security "do while" loop for bytes, interesting for busy interface or embedded systems with small network buffer
      error("Error - echo emission");
    }
    printf("Echo sent\n----------------------------\n----------------------------\n");

    return KEEP_COMMUNICATION;
  }

}

int do_socket() {
  int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);   //Sockets config: Blocking  //Possible to add SO_REUSEADDR with setsockopt() during dev phase testing...etc
  if (sockfd < 0) {
    error("Error - socket opening");
  }

  return sockfd;
}

void init_serv_address(struct sockaddr_in *serv_addr_ptr, int port_no) {
  memset(serv_addr_ptr, 0, sizeof(struct sockaddr_in));
  serv_addr_ptr->sin_family = AF_INET;
  serv_addr_ptr->sin_addr.s_addr = htonl(INADDR_ANY);  //INADDR_ANY : all interfaces - not just "localhost", multiple network interfaces OK
  serv_addr_ptr->sin_port = htons(port_no);  //convert to network order
}

void do_bind(int sockfd, struct sockaddr_in *serv_addr_ptr) {
  if ( bind(sockfd, (struct sockaddr *) serv_addr_ptr, sizeof(struct sockaddr_in))<0 ) {  //need cast generic
    error("Error - bind");
  }
}
