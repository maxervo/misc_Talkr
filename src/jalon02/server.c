#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "common.h"
#include "server.h"

int main(int argc, char* argv[]) {
  struct sockaddr_in serv_addr, cli_addr;
  int master_sockfd;
  struct Client cli_base[MAX_NO_CLI];
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
  master_sockfd = create_socket();      //add SO_REUSEADDR
  init_serv_address(&serv_addr, port_no);
  do_bind(master_sockfd, &serv_addr);

  //Client sockets
  init_client_base(cli_base);

  //Listen
  if(listen(master_sockfd, MAX_NUM_QUEUE) < 0) {
    error("Error - listen");
  }

  //Main Server Loop
  while(1) {

    //Workspace fd set preparation
    FD_ZERO(&read_fds);
    FD_SET(master_sockfd, &read_fds); //Add listening socket
    max_fd = master_sockfd;
    int i;
    for (i = 0; i < MAX_NO_CLI; i++) {  //Add all "already connected" client sockets
      if (cli_base[i].fd > EMPTY_SLOT) {  //Considering "non-error" slots (thus non negative), and "non-empty" slots
        FD_SET(cli_base[i].fd, &read_fds);
        max_fd = (cli_base[i].fd > max_fd)? cli_base[i].fd : max_fd;
      }
    }

    //Activity monitoring
    read_fds_copy = read_fds;
    if(select(max_fd + 1, &read_fds_copy, NULL, NULL, NULL) == -1) {
      error("Error - select");
    }

    //New connection
    if (FD_ISSET(master_sockfd, &read_fds_copy)) {
      new_sockfd = accept(master_sockfd, (struct sockaddr *) &cli_addr, &cli_len);

      if ( (index_available = slot_available(cli_base)) != SLOT_UNAVAILABLE ) {
        printf("New connection! Client accepted\n");
        cli_base[index_available].fd = new_sockfd;
        welcome(new_sockfd);
      }
      else {  //No more slots available, limit reached
        printf("New connection! Client refused\n");
        refuse(new_sockfd);   //Sending error msg
        close(new_sockfd);
      }
    }

    //Client already connected : IO
    else {
      int i;
      for (i = 0; i < MAX_NO_CLI; i++) {
        if (FD_ISSET(cli_base[i].fd, &read_fds_copy)) {
          if ( handle(cli_base[i].fd) == CLOSE_COMMUNICATION) {
            cli_base[i].fd = EMPTY_SLOT;
          }
        }
      }
    }
  }

  return EXIT_SUCCESS;
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

void init_client_base(struct Client *cli_base) {
  memset(cli_base, 0, MAX_NO_CLI*sizeof(struct Client));  //Considered NULL values 0, clean way would be manual init
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

int slot_available(struct Client cli_base[]) {
  int i = 0;
  while(i <= MAX_NO_CLI) {
    if (cli_base[i].fd == EMPTY_SLOT) {
      return i;
    }
    else {
      i++;
    }
  }
  return SLOT_UNAVAILABLE;
}

void welcome(int sockfd) {
  char buffer[BUFFER_SIZE];
  memset(buffer, 0, BUFFER_SIZE);
  //char welcome[] = "Welcome! Connection established with the Talkr server";

  strcpy(buffer, WELCOME_MSG);

  if (send(sockfd, buffer, BUFFER_SIZE, 0) < 0) {       //Later on: add a security "do while" loop for bytes, interesting for busy interface or embedded systems with small network buffer
    error("Error - welcome emission");
  }
}

void refuse(int sockfd) {
  char buffer[BUFFER_SIZE];
  memset(buffer, 0, BUFFER_SIZE);
  //char msg[]="Refused! Server cannot accept anymore incoming connections, retry later...\n";

  strcpy(buffer, REFUSE_MSG);

  if (send(sockfd, buffer, BUFFER_SIZE, 0) < 0) {       //Later on: add a security "do while" loop for bytes, interesting for busy interface or embedded systems with small network buffer
    error("Error - welcome emission");

  }
}
