#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h> // struct sockaddr_in

#include "common.h"

void error(const char *msg)   //ATTENTION : program flow exit
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int create_socket() {
  int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);   //Sockets config: Blocking  //Possible to add SO_REUSEADDR with setsockopt() during dev phase testing...etc
  if (sockfd < 0) {
    error("Error - socket opening");
  }

  return sockfd;
}

void do_send(int sockfd, char *buffer, int buffer_size) {
  int progress = 0; //total sent
  int sent = 0; //each try
  do {
    if ( (sent = send(sockfd, buffer+sent, buffer_size-sent, 0)) < 0 ) {
      error("Error - send");
    }
    progress += sent;
  } while(progress != buffer_size);
}

int do_recv(int sockfd, char *buffer, int buffer_size) {
  int progress = 0; //total sent
  int read = 0; //each try
  do {
    if ( (read = recv(sockfd, buffer+read, buffer_size-read, 0)) < 0 ) {
      error("Error - recv");
    }
    else if(read == 0) {  //Connection closed abruptly by remote peer, receiving 0 bytes will have the same effect
      close(sockfd);
      return CLOSE_ABRUPT;
    }
    progress += read;
  } while(progress != buffer_size);

  return progress;
}
