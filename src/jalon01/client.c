#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

#include <netdb.h>
#include <netinet/in.h> // struct sockaddr_in

#include <string.h>

#define h_addr h_addr_list[0]   /* for backward compatibility */
#define BUFFER_SIZE 256
#define QUIT_MSG "/quit\n"

void error(const char *msg);

int main(int argc, char const *argv[]) {
  int sockfd;
  int port_no;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  char buffer[BUFFER_SIZE];

  //Verify arguments
  if (argc < 3) {
    fprintf(stderr,"Program %s needs arguments regarding target server: hostname, port\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  port_no = atoi(argv[2]);

  //Open socket
  sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // IPV4 TCP, Sockets config: Blocking
  if (sockfd <0) {
    perror("Error: socket opening");
    exit(EXIT_FAILURE);
  }

  //Get server
  server = gethostbyname(argv[1]);	//maybe later use addrinfo, TODO now or not?
  if (server == NULL) {
    fprintf(stderr, "Error: No such host\n");
    exit(EXIT_FAILURE);
  }

  //Init server struct
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  memcpy(server->h_addr, &serv_addr.sin_addr.s_addr, server->h_length);  //why can't do assign direct? because of network order endian?, no need htons(ip address) ? TODO
  serv_addr.sin_port = htons(port_no);  //convert to network order

  //Connect to server
  if ( connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))<0 ) {
    perror("Error: connection");
    exit(EXIT_FAILURE);
  }
  printf("-------------------------\n- Welcome to the chat ! -\n-------------------------\n");
  printf("write '/quit' in order to close this session\n\n");
  printf("Input msg:\n ");

  while(1) {  //maybe do more elegant way
    printf("\n >>  ");
    memset(buffer, 0, BUFFER_SIZE);
    fgets(buffer, BUFFER_SIZE, stdin);

    //Send
    if (send(sockfd, buffer, BUFFER_SIZE, 0) >= 0) {
      printf("Msg sent: %s\n", buffer);
    }
    else {
      error("Error - send");
    }

    //Quit
    if (strcmp(buffer, QUIT_MSG) == 0) {
      printf("Client decided to quit the chat\n");
      return EXIT_SUCCESS;
    }

    //Receive Echo
    memset(buffer, 0, BUFFER_SIZE);
    if (recv(sockfd, buffer, BUFFER_SIZE, 0) >= 0) {    //if recv = 0, communication closed by server, supposed ok here
      printf("Echo received: %s\n", buffer);
    }
    else {
      error("Error - echo reception");
    }
  }

  return EXIT_SUCCESS;  //optional line, indeed client quits the program with "/quit" or ctrl+d
}

void error(const char *msg)   //ATTENTION : program flow exit
{
    perror(msg);
    exit(EXIT_FAILURE);
}



/*  Backup

//Write
int sent=0;
do{
  sent+= write(sockfd, buffer+sent, 20-sent);		//maybe encompass it in while security loop, or use send...etc
} while (sent!=20);


printf("\nMsg sent: %s", buffer);
if (strcmp(buffer, QUIT_MSG)<=0)
  exit(EXIT_SUCCESS);

//Read echo
memset(buffer, 0, BUFFER_SIZE);
int readen=0;
do{
  readen+= read(sockfd, buffer+readen, 20-readen);		//maybe encompass it in while security loop, or use send...etc
} while (readen!=20);
buffer[readen+1] = '\0'; // vu sur developpez.com

printf("\nEcho received: %s", buffer);
}


return EXIT_SUCCESS;

*/
