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
  int master_sockfd, cli_sock[MAX_NO_CLI];
  int new_sockfd;
  fd_set read_fds;
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
  do_bind(sockfd, &serv_addr);

  //Client sockets
  for(int i=0; i < MAX_NO_CLI; i++) {
    cli_sock[i] = 0;
  }

  //Listen
  if(listen(sockfd, MAX_NUM_QUEUE) < 0) {
    error("Error - listen");
  }


  while(1) {

    //Workspace fd set preparation
    FD_ZERO(&read_fds);
    FD_SET(master_sockfd, &read_fds);
    max_fd = master_sockfd;
    for (int i = 0; i < MAX_NO_CLI; i++) {
      if (cli_sock[i] > 0) {
        FD_SET(cli_sock[i], &read_fds);
        max_fd = (cli_sock[i] > max_fd)? cli_sock[i] : max_fd;
      }
    }

    if(select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1) {
      error("Error - select")
    }

    //New connection
    if (FD_ISSET(master_sockfd, &read_fds)) {
      new_sockfd = accept(master_sockfd, (struct sockaddr *) &cli_addr, &cli_len)
      if (index_available = slotfd_available(cli_sock)) {
        cli_sock[index_available] = new_sockfd;

        //do stuff send stuff welcome
      }
      else {  // no more slots available, limit reached
        //send msg error
        close(new_sockfd);
      }
    }

    //Already client IO
    else {
      for (int i = 0; i < MAX_NO_CLI; i++) {
        if (FD_ISSET(cli_sock[i], &read_fds)) {
          //read what he sent, or close connection...etc
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
