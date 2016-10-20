#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "server.h"

int main(int argc, char* argv[]) {
  struct sockaddr_in serv_addr, cli_addr;
  int master_sockfd;
  struct client cli_base[MAX_NO_CLI];
  int new_sockfd;
  fd_set read_fds, read_fds_copy; //copy because of select -> clean
  int max_fd;
  int index_available;

  int cli_len = sizeof(cli_addr);
  int port_no;

  //Check Usage
  if (argc != 2) {
      fprintf(stderr, "Usage: RE216_SERVER port\n");
      return EXIT_FAILURE;
  }
  port_no = atoi(argv[1]);

  //Preparing server
  master_sockfd = do_socket();      //add SO_REUSEADDR
  init_serv_address(&serv_addr, port_no);
  do_bind(master_sockfd, &serv_addr);

  //Client sockets
  init_client_base(cli_base);

  //Listen
  if(listen(master_sockfd, MAX_NUM_QUEUE) < 0) {
    error("Error - listen");
  }


  while(1) {

    //Workspace fd set preparation
    FD_ZERO(&read_fds);
    FD_SET(master_sockfd, &read_fds); // listen socket
    max_fd = master_sockfd;
    int i;
    for (i = 0; i < MAX_NO_CLI; i++) {
      if (cli_base[i].fd > 0) {
        FD_SET(cli_base[i].fd, &read_fds);
        max_fd = (cli_base[i].fd > max_fd)? cli_base[i].fd : max_fd;
      }
    }

    read_fds_copy = read_fds;
    if(select(max_fd + 1, &read_fds_copy, NULL, NULL, NULL) == -1) {
      error("Error - select");
    }

    //New connection
    if (FD_ISSET(master_sockfd, &read_fds_copy)) {
      new_sockfd = accept(master_sockfd, (struct sockaddr *) &cli_addr, &cli_len);

      if ( (index_available = slotfd_available(cli_base)) != SLOTFD_UNAVAILABLE ) {
        printf("Client accepted\n");
        cli_base[index_available].fd = new_sockfd;
        welcome(new_sockfd);
      }
      else {  // no more slots available, limit reached
        printf("Client refused\n");
        refuse(new_sockfd);//send msg error
        close(new_sockfd);
      }
    }

    //Already client IO
    else {
      int i;
      for (i = 0; i < MAX_NO_CLI; i++) {
        if (FD_ISSET(cli_base[i].fd, &read_fds_copy)) {
          if ( CLOSE_COMMUNICATION == handle(cli_base[i].fd) ) {
            cli_base[i].fd=0;
          }
        }
      }
    }
  }

  /*
  //Server Loop : Acceptance Queue
  for (int i=0; i < MAX_NO_CONNECTIONS; i++) {
    new_sockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &cli_len);   //Blocked, (Sockets config: Blocking), so only one connection opened at a time
    if (new_sockfd < 0) {
      error("Error - accept");
    }
    while(handle(new_sockfd)){     //while the connection is open
      //Handling each time
    }
    close(new_sockfd);
  }
  close(sockfd);*/

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
    printf("[%i]: %s",sockfd, buffer);

    //Echo
    if (send(sockfd, buffer, BUFFER_SIZE, 0) < 0) {       //Later on: add a security "do while" loop for bytes, interesting for busy interface or embedded systems with small network buffer
      error("Error - echo emission");
    }
    printf("Echo sent to [%i]\n----------------------------\n----------------------------\n", sockfd);

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

int slotfd_available(struct client cli_base[]) {
  int i = 0;
  while(i <= MAX_NO_CLI) {
    if (cli_base[i].fd == 0) {
      return i;
    }
    else {
      i++;
    }
  }
  return SLOTFD_UNAVAILABLE;
}

int welcome(int sockfd) {
  char buffer[BUFFER_SIZE];
  memset(buffer, 0, BUFFER_SIZE);
  char * welcome="-------------------------\n- Welcome to the chat ! -\n-------------------------\n write '/quit' in order to close this session\n\n Input msg:\n ";

  strcpy(buffer,welcome);

  if (send(sockfd, buffer, BUFFER_SIZE, 0) < 0) {       //Later on: add a security "do while" loop for bytes, interesting for busy interface or embedded systems with small network buffer
    error("Error - welcome emission");
  }
  printf("----------------------------\n------New Connection !------\n----------------------------\n");

  return KEEP_COMMUNICATION;
}

int refuse(int sockfd) {
  char buffer[BUFFER_SIZE];
  memset(buffer, 0, BUFFER_SIZE);
  char * msg="\n-------------------------\n--  Retry again later ! --\n-------------------------\n (Server cannot accept incoming connections anymore)\n\n";

  strcpy(buffer,msg);

  if (send(sockfd, buffer, BUFFER_SIZE, 0) < 0) {       //Later on: add a security "do while" loop for bytes, interesting for busy interface or embedded systems with small network buffer
    error("Error - welcome emission");

  }
  printf("----------------------------\n--New connection refused !--\n----------------------------\n");
  return KEEP_COMMUNICATION;
}

void init_client_base(struct client *cli_base) {
  memset(cli_base, 0, MAX_NO_CLI*sizeof(struct client));  //Considered NULL values 0, clean way would be manual init
}
