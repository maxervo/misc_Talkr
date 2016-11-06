#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h> // struct sockaddr_in
#include <sys/select.h>

#include "common.h"
#include "client.h"


int main(int argc, char const *argv[]) {
  char alias[ALIAS_SIZE];
  int srv_sockfd;
  int port_no;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  char buffer[BUFFER_SIZE];
  fd_set read_fds, read_fds_copy;   //copy because of select, to be clean

  //Verifying arguments
  if (argc < 3) {
    fprintf(stderr,"Program %s needs arguments regarding target server: hostname, port\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  port_no = atoi(argv[2]);

  // Welcome msg
  printf("-------------------------\n- Talkr Client ! -\n-------------------------\n");
  printf("Write '/quit' in order to close this session\n");

  //Preparing
  srv_sockfd = create_socket();
  server = get_server(argv[1]);
  init_serv_address(server, &serv_addr, port_no);

  //Connect to server
  if ( connect(srv_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))<0 ) {
    error("Error - connection");
  }

  //Receive Welcome/Refuse msg
  memset(buffer, 0, BUFFER_SIZE);
  do_recv(srv_sockfd, buffer, BUFFER_SIZE);
  printf("%s\n", buffer);
  if (strcmp(buffer, REFUSE_MSG) == 0){
    error("Error - connection");
  }

  // set nickname
  strcpy(alias,"NO_NICKNAME_YET");

  //Main Client Loop
  while(1) {

    //Workspace fd set preparation
    FD_ZERO(&read_fds);
    int max_fd=srv_sockfd+1;
    FD_SET(0, &read_fds);
    FD_SET(srv_sockfd, &read_fds);
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    //Activity monitoring
    read_fds_copy = read_fds;
    if(select(max_fd + 1, &read_fds_copy, NULL, NULL, &tv) == -1) {
      error("Error - select");
    }

    // Msg from server
    if(FD_ISSET(srv_sockfd, &read_fds_copy)){
      //Receive Echo
      memset(buffer, 0, BUFFER_SIZE);
      do_recv(srv_sockfd, buffer, BUFFER_SIZE);
      printf("Msg received: %s\n", buffer);

      if (strncmp(buffer,ALIAS_MSG,sizeof(ALIAS_MSG)-1)==0) {
        set_alias(buffer,alias);

      }

    }
    if(FD_ISSET(0, &read_fds_copy)){
      char msg_to_send[BUFFER_SIZE-ALIAS_SIZE];
      memset(buffer, 0, BUFFER_SIZE);
      fgets(msg_to_send, BUFFER_SIZE-ALIAS_SIZE, stdin);

      printf("Alias %s\n", alias);
      strcat(buffer, "[");
      strcat(buffer, alias);
      strcat(buffer, "]   ");
      strcat(buffer, msg_to_send );

      do_send(srv_sockfd, buffer, BUFFER_SIZE);
      printf("Msg sent : %s", buffer);

      //Quit
      if (strcmp(buffer, QUIT_MSG) == 0) {
        printf("Client decided to quit the chat\n");
        close(srv_sockfd);
        return EXIT_SUCCESS;
      }
    }

  }

  return EXIT_SUCCESS;  //optional line, indeed client quits the program with "/quit" or ctrl+c
}


struct hostent* get_server(const char *host_target) {
  struct hostent *server = gethostbyname(host_target);	//Later on: use addrinfo (cf. gethostbyname considered deprecated, and for ipv6...etc)
  if (server == NULL) {
    fprintf(stderr, "Error: No such host\n");
    exit(EXIT_FAILURE);
  }

  return server;
}

void init_serv_address(struct hostent* server, struct sockaddr_in* serv_addr_ptr, int port_no) {
  memset(serv_addr_ptr, 0, sizeof(struct sockaddr_in));
  serv_addr_ptr->sin_family = AF_INET;
  memcpy(server->h_addr, &(serv_addr_ptr->sin_addr.s_addr), server->h_length);
  serv_addr_ptr->sin_port = htons(port_no);  //convert to network order
}

void set_alias(char *buffer,char *alias){
  int i=0;
  const char arrow[3] = "->";
  char *token;

  token = strtok(buffer, arrow);
  token = strtok(NULL, arrow);
  strcpy(alias, token);
  while (alias[i]!='\n') {
     i++;
  }
  alias[i-1]='\0';
  alias[sizeof(alias)-1] = '\0';
}
