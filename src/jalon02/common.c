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
