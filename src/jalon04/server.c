#include <stdio.h>      //Later on, do libevent for asynchronous event based server
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#include "common.h"
#include "server.h"

int main(int argc, char* argv[]) {
  struct sockaddr_in serv_addr, cli_addr;
  int master_sockfd;
  struct Client cli_base[MAX_NO_CLI];   //array chosen instead of lists because: max size known
  int new_sockfd;
  fd_set read_fds, read_fds_copy;   //copy because of select, to be clean
  int max_fd;
  int index_available;

  int cli_len = sizeof(cli_addr);
  int port_no;

  struct timeval tv;    //clean select
  tv.tv_sec = 0;
  tv.tv_usec = 0;

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

  printf("-------------------------\n- Talkr Client ! -\n-------------------------\n");
  printf("Server running ....\n\n\n");

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
    if(select(max_fd + 1, &read_fds_copy, NULL, NULL, &tv) == -1) {
      error("Error - select");
    }

    //New connection
    if (FD_ISSET(master_sockfd, &read_fds_copy)) {
      new_sockfd = accept(master_sockfd, (struct sockaddr *) &cli_addr, &cli_len);

      if ( (index_available = slot_available(cli_base)) != SLOT_UNAVAILABLE ) {
        printf("New connection! Client accepted\n\n");
        set_client(&cli_base[index_available], new_sockfd, cli_addr);
        welcome(new_sockfd); //Sending welcome msg
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
          if ( handle(&cli_base[i], cli_base) == CLOSE_COMMUNICATION) {
            reset_client_slot(cli_base+i);
          }
        }
      }
    }
  }

  return EXIT_SUCCESS;
}

int handle(struct Client *client, struct Client *cli_base) {
  int sockfd = client->fd;
  char alias[ALIAS_SIZE];
  strncpy(alias,client->alias,ALIAS_SIZE);
  char buffer_serv[BUFFER_SERV_SIZE];
  memset(buffer_serv, 0, BUFFER_SERV_SIZE);
  //char buffer_serv_copy[BUFFER_SERV_SIZE];            // to extract token, we work only on a copy because strtok modifies the input
  //memset(buffer_serv_copy, 0, BUFFER_SERV_SIZE);
  char buffer_cli[BUFFER_CLI_SIZE];
  memset(buffer_cli, 0, BUFFER_CLI_SIZE);

  const char space[2] = " ";
  char token_cmd[CMD_SIZE] = {'\0'}; // can be /quit /nick /whois /who
  char token_arg[ARG_SIZE] = {'\0'}; // for /nick & /whois
  char token_data[BUFFER_SERV_SIZE] = {'\0'};

  //Abrupt close by client
  if( do_recv(sockfd, buffer_serv, BUFFER_SERV_SIZE) == CLOSE_ABRUPT ) {
    printf("Connection closed abruptly by remote peer\n");
    return CLOSE_COMMUNICATION;
  }
  else { // extract the first word of the msg receive

    printf("BEFORE\n");
    parse_request(buffer_serv, token_cmd, token_arg, token_data);
    printf("AFTER %s and %s and %s\n", token_cmd, token_arg, token_data);
    /*
    strncpy(buffer_serv_copy, buffer_serv, BUFFER_SERV_SIZE);
    token_cmd = strtok(buffer_serv_copy, space);   //can be /quit /nick /whois /who
    token_arg = strtok(NULL, space);*/

    /*
    // Walk through the leftovers
    while( token != NULL ){
      token = strtok(NULL, space);
      strcat(buffer_cli,token);
    }*/
  }

  //Quit
  if(strcmp(token_cmd,QUIT_MSG) == 0) {
    printf("Quit msg received : connection closed by client\n");
    close(sockfd);
    return CLOSE_COMMUNICATION;
  }

  //Set nickname
  else if(strcmp(token_cmd,NICK_MSG) == 0) {
    //token_arg = strtok(NULL, space);

    printf("Nick msg received \n");

    if (token_arg != NULL && strlen(token_arg) < ALIAS_SIZE+1) {  //+1 for the \n
      token_arg[strlen(token_arg)-1] = '\0';   //remove \n
      set_nickname(client, token_arg, cli_base);
    }
    else {
      printf("Incorrect alias\n");  //non existent or too long
      inform_alias_incorrect(sockfd);
    }

    return KEEP_COMMUNICATION;
  }

  //Demand alias again
  else if(strlen(client->alias) == 0) {
    printf("Client has not set his nickname yet\n");
    inform_nick_demand(sockfd);
    return KEEP_COMMUNICATION;
  }

  //Whois
  else if(strcmp(token_cmd,WHOIS_MSG) == 0) {
    printf("Whois msg received\n");

    if (token_arg != NULL && strlen(token_arg) < ALIAS_SIZE+1) {  //+1 for the \n
      token_arg[strlen(token_arg)-1] = '\0';   //remove \n
      inform_whois(client, token_arg, cli_base);
    }
    else {
      printf("Alias incorrect\n"); //non existent or too long
      inform_alias_incorrect(sockfd);
    }

    return KEEP_COMMUNICATION;
  }

  //Who
  else if(strcmp(token_cmd,WHO_MSG) == 0) {
    printf("Who msg received\n");
    inform_who(sockfd, cli_base);
    return KEEP_COMMUNICATION;
  }

  // Broadcast
  else if(strcmp(token_cmd, BROADCAST_MSG) == 0) {
    printf("Broadcast msg received\n");
    broadcast(sockfd, cli_base, buffer_cli);
    return KEEP_COMMUNICATION;
  }

  // Unicast
  else if(strcmp(token_cmd,UNICAST_MSG) == 0) {
    printf("Unicast msg received\n");
    unicast(sockfd, cli_base, buffer_serv, token_arg);
    return KEEP_COMMUNICATION;
  }

  //Simple msg : TODO for chatrooms
  else {
    strncpy(buffer_cli, buffer_serv, BUFFER_SERV_SIZE);
    printf("Sending to client with fd [%i]: %s", sockfd, buffer_cli);
    do_send(sockfd, buffer_cli, BUFFER_CLI_SIZE);
    printf("Echo sent\n\n");

    return KEEP_COMMUNICATION;
  }

}

void init_client_base(struct Client *cli_base) {
  for(int i=0; i < MAX_NO_CLI; i++) {
    cli_base[i].fd = EMPTY_SLOT;
    cli_base[i].alias[0] = '\0';
    cli_base[i].con_time[0] = '\0';
    cli_base[i].ip[0] = '\0';
    cli_base[i].port = 0;
    cli_base[i].room[0] = '\0';
  }
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
  char buffer[BUFFER_CLI_SIZE];
  memset(buffer, 0, BUFFER_CLI_SIZE);
  strcpy(buffer, WELCOME_MSG);

  do_send(sockfd, buffer, BUFFER_CLI_SIZE);
}

void inform_nick_set(int sockfd, char* alias) {
  char buffer[BUFFER_CLI_SIZE];
  memset(buffer, 0, BUFFER_CLI_SIZE);
  strcat(buffer, ALIAS_MSG);
  strcat(buffer, alias);
  strcat(buffer, " \n");
  do_send(sockfd, buffer, BUFFER_CLI_SIZE);
}

void inform_nick_demand(int sockfd) {
  char buffer[BUFFER_CLI_SIZE];
  memset(buffer, 0, BUFFER_CLI_SIZE);
  strcpy(buffer, "[SERVER] Please set a nickname first with /nick\n");

  do_send(sockfd, buffer, BUFFER_CLI_SIZE);
}

void refuse(int sockfd) {
  char buffer[BUFFER_CLI_SIZE];
  memset(buffer, 0, BUFFER_CLI_SIZE);
  strcpy(buffer, REFUSE_MSG);

  do_send(sockfd, buffer, BUFFER_CLI_SIZE);
}

void set_nickname(struct Client *client, char *alias, struct Client *cli_base){
  printf("Set nickname : %s\n", alias);

  if (!presence_alias(alias, cli_base)) {
    strncpy(client->alias, alias, ALIAS_SIZE);
    inform_nick_set(client->fd, alias);
    printf("Nickname set\n");
  }
  else {
    printf("Nickname already used by another client\n");
    inform_nick_used(client->fd);
  }

}

void inform_nick_used(int sockfd) {
  char buffer[BUFFER_CLI_SIZE];
  memset(buffer, 0, BUFFER_CLI_SIZE);
  strcpy(buffer, "[SERVER] Nickname already used by another client\n");

  do_send(sockfd, buffer, BUFFER_CLI_SIZE);
}

int presence_alias(char *alias, struct Client *cli_base) {
  for(int i=0; i < MAX_NO_CLI; i++) {
    if(strcmp(alias, cli_base[i].alias) == 0) {
      return 1;
    }
  }
  return 0;
}

void reset_client_slot(struct Client *client) {
  client->fd = EMPTY_SLOT;
  client->alias[0] = '\0';
  client->con_time [0] = '\0';
  client->ip[0] = '\0';
  client->port = 0;
}

void inform_who(int sockfd,struct Client *cli_base) {
  char buffer[BUFFER_CLI_SIZE];
  memset(buffer, 0, BUFFER_CLI_SIZE);
  strcat(buffer, "\n");
  for (int i = 0; i < MAX_NO_CLI; i++) {
    if (cli_base[i].fd != EMPTY_SLOT) {
      strcat(buffer, cli_base[i].alias);
      strcat(buffer, "\n");
    }
  }

  do_send(sockfd, buffer, BUFFER_CLI_SIZE);
}

void set_client(struct Client* client, int new_sockfd, struct sockaddr_in cli_addr) {

  //connection time
  time_t current_time = time(NULL);
  struct tm *info = localtime(&current_time);
  strftime(client->con_time, CON_TIME_SIZE, "%d/%m/%Y %H:%M:%S", info);

  //fd
  client->fd = new_sockfd;

  //ip address
  strncpy(client->ip, inet_ntoa(cli_addr.sin_addr), IP_SIZE);
  client->port = cli_addr.sin_port;

  printf("%s and %d\n", client->ip, client->port);

}

void inform_whois(struct Client *client, char *alias, struct Client *cli_base) {
  char str_port[10] = {0};
  printf("Search nickname : %s\n", alias);

  char buffer[BUFFER_CLI_SIZE];
  memset(buffer, 0, BUFFER_CLI_SIZE);
  strcat(buffer, "\n ");

  //Searching client by alias
  for (int i = 0; i < MAX_NO_CLI; i++) {
    if (strcmp(cli_base[i].alias, alias) == 0 ) {       //TODO do with snprintf
      strcat(buffer, "[SERVER] User ");
      strcat(buffer, cli_base[i].alias);
      strcat(buffer, " connected since ");
      strcat(buffer, cli_base[i].con_time);
      strcat(buffer, " with ip address ");
      strcat(buffer, cli_base[i].ip);
      strcat(buffer, " and port number ");
      sprintf(str_port, "%d", cli_base[i].port);
      strcat(buffer, str_port);
      strcat(buffer, "\n");

      do_send(client->fd, buffer, BUFFER_CLI_SIZE);
      return;
    }
  }

  //Client not found
  strcat(buffer, "[SERVER] Client not found, unkown alias.\n");
  do_send(client->fd, buffer, BUFFER_CLI_SIZE);

}

void inform_alias_incorrect(int sockfd) {
  char buffer[BUFFER_CLI_SIZE];
  memset(buffer, 0, BUFFER_CLI_SIZE);
  strcpy(buffer, "[SERVER] Alias incorrect\n");

  do_send(sockfd, buffer, BUFFER_CLI_SIZE);
}

void broadcast(int sockfd,struct Client *cli_base, char*buffer){
  // Client buffer is higher than server buffer
  char buffer_cli[BUFFER_CLI_SIZE];
  const char space[2] = "-";
  char *token;
  memset(buffer_cli, 0, BUFFER_CLI_SIZE);

  // Knowing who sent the message and what type the message is (broadcast)
  strcat(buffer_cli, "[");
  strcat(buffer_cli, "YourNickname");
  strcat(buffer_cli, "] [BRAODCAST] :");

  /* get the first token to take of */
  token = strtok(buffer, space); // token = "/msgall" here

  /* walk through other tokens */
  while( token != NULL ){
    token = strtok(NULL, space);
    strcat(buffer_cli,token);
  }

  for (int i=0;i<MAX_NO_CLI;i++){
    if(cli_base[i].fd != EMPTY_SLOT && cli_base[i].fd != sockfd ){
      do_send(cli_base[i].fd, buffer_cli, BUFFER_CLI_SIZE);
    }
  }

}

void unicast(int sockfd, struct Client *cli_base, char *msg, char *alias_receiver) {

  char buffer_cli[BUFFER_CLI_SIZE] = {'\0'}; // Client buffer is bigger than server buffer
  char* alias_sender = get_alias_from_fd(sockfd, cli_base);
  int fd_receiver = get_fd_from_alias(alias_receiver, cli_base);

  snprintf(buffer_cli, BUFFER_CLI_SIZE, "[%s] -> %s", alias_sender, msg);

  if (presence_alias(alias_receiver, cli_base)) {
    do_send(fd_receiver, buffer_cli, BUFFER_CLI_SIZE);
  }
  else {
    inform_alias_incorrect(sockfd);
  }


  //const char space[2] = "-";
  //char *token;
  //memset(buffer_cli, 0, BUFFER_CLI_SIZE);

  /*
  // this part allow to know who send the message and what type the message is (broadcast)
  strcat(buffer_cli, "[");
  strcat(buffer_cli, "YourNickname");
  strcat(buffer_cli, "] [UNICAST] :");

  /* get the 2 first token to take of */
  //token = strtok(buffer, space); // token = "/msgall" here
  //token = strtok(NULL, space); // token = nickname

  /* walk through other tokens */
  /*
  while( token != NULL ){
    token = strtok(NULL, space);
    strcat(buffer_cli,token);
  }*/

  /*
  printf("L'alias demandé est : %s\n", alias);
  for (i=0;i<MAX_NO_CLI;i++){
    if((strcmp(alias,cli_base[i].alias)==0) && cli_base[i].fd!=sockfd ){
      printf("L'alias trouvé est : %s\n", cli_base[i].alias);
      printf("L'alias trouvé est : %i\n", cli_base[i].fd);
      do_send(cli_base[i].fd, buffer_cli, BUFFER_CLI_SIZE);
      break;
    }
  }*/
}

void parse_request(char *buffer_serv, char *token_cmd, char *token_arg, char *token_data) {
  char buffer_serv_copy[BUFFER_SERV_SIZE];            // to extract token, we work only on a copy because strtok modifies the input
  memset(buffer_serv_copy, 0, BUFFER_SERV_SIZE);
  strncpy(buffer_serv_copy, buffer_serv, BUFFER_SERV_SIZE);   //to be clean when extracting tokens
  const char space[2] = " ";
  char* token = strtok(buffer_serv_copy, space);   //can be /quit /nick /whois /who  /msgall /msg

  //QUIT or WHO
  if ( (strcmp(token, QUIT_MSG) == 0) || (strcmp(token, WHOIS_MSG) == 0) ) {
    strncat(token_cmd, token, CMD_SIZE);
    return;
  }

  //WHOIS or NICK
  else if(strcmp(token, WHOIS_MSG) == 0 || strcmp(token, NICK_MSG) == 0) {
    strncat(token_cmd, token, CMD_SIZE);
    strncat(token_arg, strtok(NULL, space), ARG_SIZE);

    return;
  }

  //MSGALL
  else if(strcmp(token, BROADCAST_MSG) == 0) {
    strncat(token_cmd, token, CMD_SIZE);

    // Walk through the data
    while( token != NULL ){
      token = strtok(NULL, space);
      if (token != NULL) {strncat(token_data, token, BUFFER_CLI_SIZE);}
    }

    return;
  }

  //UNICAST MSG
  else if(strcmp(token, UNICAST_MSG) == 0) {
    strncat(token_cmd, token, CMD_SIZE);
    strncat(token_arg, strtok(NULL, space), ARG_SIZE);

    // Walk through the data
    while( token != NULL ){
      printf("before token\n");
      token = strtok(NULL, space);
      printf("It's %s\n", token);
      if (token != NULL) {strncat(token_data, token, BUFFER_CLI_SIZE);}
    }

    return;
  }

}

char *get_alias_from_fd(int fd, struct Client *cli_base) {       //For production purpose and better design, hash tables can be implemented to have fd as index
  for(int i=0; i < MAX_NO_CLI; i++) {
    if(fd == cli_base[i].fd) {
      return cli_base[i].alias;
    }
  }
  return NULL;
}

int get_fd_from_alias(char *alias, struct Client *cli_base) {
  for(int i=0; i < MAX_NO_CLI; i++) {
    if(strcmp(alias, cli_base[i].alias) == 0) {
      return cli_base[i].fd;
    }
  }
  return EMPTY_SLOT;
}
