#include <stdio.h>      //Later on, do libevent for asynchronous event based server
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
  struct Client cli_base[MAX_NO_CLI];   //array chosen instead of lists because: max size known
  int new_sockfd;
  fd_set read_fds, read_fds_copy; //copy because of select, to be clean
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
  master_sockfd = create_socket();
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
        printf("New connection! Client accepted\n\n");
        cli_base[index_available].fd = new_sockfd;
        welcome(new_sockfd);
      }
      else {  //No more slots available, limit reached
        printf("New connection! Client refused\n\n");
        refuse(new_sockfd);   //Sending error msg
        close(new_sockfd);
      }
    }

    //Client already connected : IO
    else {
      int i;
      for (i = 0; i < MAX_NO_CLI; i++) {
        if (FD_ISSET(cli_base[i].fd, &read_fds_copy)) {
          if ( handle(&cli_base[i]) == CLOSE_COMMUNICATION) {
            cli_base[i].fd = EMPTY_SLOT;
          }
        }
      }
    }
  }

  return EXIT_SUCCESS;
}

int handle(struct Client *cli_base) {
  int sockfd=cli_base->fd;
  char alias[ALIAS_SIZE];
  strncpy(alias,cli_base->alias,ALIAS_SIZE);
  char buffer[BUFFER_SIZE];
  memset(buffer, 0, BUFFER_SIZE);

  const char space[2] = "";
  char *first_token= strtok(buffer, space); // can be /quit /nick /whois /who
  printf("%s\n",first_token );

  //Abrupt close by client
  if( do_recv(sockfd, buffer, BUFFER_SIZE) == CLOSE_ABRUPT ) {
    printf("Connection closed abruptly by remote peer\n");
    return CLOSE_COMMUNICATION;
  }
  else{ // extract the first word of the msg receive
    const char space[2] = "";
    char *first_token;
    printf("%s\n",buffer );
    first_token= strtok(buffer, space); // can be /quit /nick /whois /who
    printf("%s\n",first_token );
  }

  //Quit
  if(strcmp(buffer,QUIT_MSG) == 0) {
    printf("Quit msg received : connection closed by client\n");
    close(sockfd);
    return CLOSE_COMMUNICATION;
  }

  //set nickname
  else if(strcmp(first_token,NICK_MSG) == 0) {
    printf("Nick msg received \n");
    set_nickname(cli_base,buffer);
    do_send(sockfd, buffer, BUFFER_SIZE);
    printf("Echo sent\n\n");
    return KEEP_COMMUNICATION;
  }

  // No nickname
  /*else if(strcmp(alias,NULL)) {
    printf("No nickname set yet \n");
  }*/

  //Msg
  else {
    printf("Sending to client with fd [%i]: %s", sockfd, buffer);
    do_send(sockfd, buffer, BUFFER_SIZE);
    printf("Echo sent\n\n");
    printf("%s",alias);

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
  if ( bind(sockfd, (struct sockaddr *) serv_addr_ptr, sizeof(struct sockaddr_in))<0 ) {  //cast generic struct
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
  strcpy(buffer, WELCOME_MSG);

  do_send(sockfd, buffer, BUFFER_SIZE);
}

void refuse(int sockfd) {
  char buffer[BUFFER_SIZE];
  memset(buffer, 0, BUFFER_SIZE);
  strcpy(buffer, REFUSE_MSG);

  do_send(sockfd, buffer, BUFFER_SIZE);
}

void set_nickname(struct Client *cli_base,char * buffer){
  const char s[2] = "";
  char *token;

  /* get the first token */
  token = strtok(buffer, s);
  token = strtok(buffer, s);
  printf("Set nickname : %s\n",token );
  strncpy(cli_base->alias,token, ALIAS_SIZE);// core dumped
  printf("Set nickname : %s\n",cli_base->alias );

  //strncpy("pouet",cli_base->alias,sizeof(ALIAS_SIZE)); core dumped
}
