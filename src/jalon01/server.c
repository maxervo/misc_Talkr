#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_CONC_CONNECTIONS 20
#define MAX_NUM_QUEUE 10
#define BUFFER_SIZE 256
#define QUIT_MSG "\\quit"

void error(const char *msg);
int handle(int sockfd);

int main(int argc, char* argv[]) {
  struct sockaddr_in serv_addr, cli_addr;
  int sockfd, new_sockfd;
  int cli_len = sizeof(cli_addr);
  int port_no;


  if (argc != 2)
  {
      fprintf(stderr, "Usage: RE216_SERVER port\n");
      return EXIT_FAILURE;
  }
  port_no = atoi(argv[1]);

  //Open socket
  sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sockfd < 0) {
    error("Error: socket opening");
  }

  //Init socket struct
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  //convert to network order
  serv_addr.sin_port = htons(port_no);  //convert to network order

  //Bind
  if ( bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))<0 ) {  //need cast generic
    error("Error: bind");
  }

  //Listen
  listen(sockfd, MAX_NUM_QUEUE);

  //Accept Queue
  while(1){
    int i;
    for (i=0; i < MAX_CONC_CONNECTIONS; i++)
    {
      new_sockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &cli_len);
      if (new_sockfd < 0) {
        error("Error: accept");
      }
      while(handle(new_sockfd)){          // while the connection is open
      //handle(new_sockfd);
      }
      close(new_sockfd);
    }
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

  //Read
  int readen=0;
  do{
    readen+= read(sockfd, buffer+readen, 20-readen);		//maybe encompass it in while security loop, or use send...etc
  } while (readen!=20);

  char quit[6]; // \\quit
  strncpy(quit,buffer,6);
  buffer[readen+1] = '\0'; // vu sur developpez.com

  if (strcmp(quit,QUIT_MSG) <= 0) {
    printf("Connection closed by the client\n");
    close(sockfd);
    return 0;
  }
  else {
    printf("msg received : %s", buffer);
  }

  //Echo
  int sent=0;
  do{
    sent+= write(sockfd, buffer+sent, 20-sent);		//maybe encompass it in while security loop, or use send...etc
  } while (sent!=20);

  printf("Echo sent\n----------------------------\n----------------------------\n");
  return 1;
}
