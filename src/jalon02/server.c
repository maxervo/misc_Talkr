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
  int i;
  for(i=0; i < MAX_NO_CLI; i++) {
    cli_sock[i] = 0;
  }

  //Listen
  if(listen(master_sockfd, MAX_NUM_QUEUE) < 0) {
    error("Error - listen");
  }


  while(1) {

    //Workspace fd set preparation
    FD_ZERO(&read_fds); // mise a zéro
    FD_SET(master_sockfd, &read_fds); // listen Socket
    max_fd = master_sockfd;
    int i;
    for (i = 0; i < MAX_NO_CLI; i++) {
      if (cli_sock[i] > 0) {
        FD_SET(cli_sock[i], &read_fds);
        max_fd = (cli_sock[i] > max_fd)? cli_sock[i] : max_fd;
      }
    }

    read_fds_copy = read_fds;
    if(select(max_fd + 1, &read_fds_copy, NULL, NULL, NULL) == -1) {
      error("Error - select");
    }

    //New connection
    if (FD_ISSET(master_sockfd, &read_fds_copy)) {
      new_sockfd = accept(master_sockfd, (struct sockaddr *) &cli_addr, &cli_len);

      if (-1!=(index_available = slotfd_available(cli_sock))) {
        printf("Client accepté\n");
        cli_sock[index_available] = new_sockfd;
        welcome(new_sockfd);
        //do stuff send stuff welcome
      }
      else {  // no more slots available, limit reached
        printf("Client refusé\n");
        NoSlotAvailable(new_sockfd);//send msg error
        close(new_sockfd);
      }
    }

    //Already client IO
    else {
      int i;
      for (i = 0; i < MAX_NO_CLI; i++) {
        if (FD_ISSET(cli_sock[i], &read_fds_copy)) {
          if (CLOSE_COMMUNICATION==handle(cli_sock[i])) {
            cli_sock[i]=0;
          }
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
