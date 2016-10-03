#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) {
  struct sockaddr_in serv_addr, cli_addr;
  int sockfd, new_sockfd;
  int cli_len = sizeof(cli_addr);
  int port_no;
  pid_t pid;

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
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(port_no);  //convert to network order

  //Bind
  if ( bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))<0 ) {  //need cast generic
    error("Error: bind");
  }

  //Listen
  listen(sockfd, MAX_NUM_QUEUE);

  //Loop
  while (1) {

    //Accept
    new_sockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &cli_len);
    if (new_sockfd < 0) {
      error("Error: accept");
    }

    //Fork
    pid = fork();
    if (pid < 0) {
      error("Error: fork");
    }

    //In child process
    if (pid == 0) {
      close(sockfd);
      handle(new_sockfd);
      exit(EXIT_SUCCESS);
    }

    //Still in parent process
    else {
      close(new_sockfd);
    }

  } //end of loop

  return EXIT_SUCCESS;
}

void handle(int sockfd) {
  char buffer[BUFFER_SIZE];
  memset(buffer, 0, BUFFER_SIZE);

  //Read
  if (read(sockfd, buffer, 32) < 0) {
    error("Error: read");
  }
  printf("msg received : %s\n", buffer);

  //Echo
  if (write(sockfd, buffer, 32) < 0) {
    error("Error: write echo");
  }
  printf("Echo sent\n");


}
{

    if (argc != 2)
    {
        fprintf(stderr, "usage: RE216_SERVER port\n");
        return 1;
    }


    //create the socket, check for validity!
    //do_socket()


    //init the serv_add structure
    //init_serv_addr()

    //perform the binding
    //we bind on the tcp port specified
    //do_bind()

    //specify the socket to be a server socket and listen for at most 20 concurrent client
    //listen()

    for (;;)
    {

        //accept connection from client
        //do_accept()

        //read what the client has to say
        //do_read()

        //we write back to the client
        //do_write()

        //clean up client socket
    }

    //clean up server socket

    return 0;
}
