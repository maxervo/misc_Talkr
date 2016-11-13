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
  struct Channel channel_base[MAX_NO_CHANNEL];
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

  //Init bases
  init_client_base(cli_base);
  init_channel_base(channel_base);

  //Listen
  if(listen(master_sockfd, MAX_NUM_QUEUE) < 0) {
    error("Error - listen");
  }

  printf("-------------------------\n-    Talkr Client !    -\n-------------------------\n");
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
          if ( handle(&cli_base[i], cli_base, channel_base) == CLOSE_COMMUNICATION) {
            clean_channel(&cli_base[i], channel_base);
            reset_client_slot(cli_base+i);
          }
        }
      }
    }
  }

  return EXIT_SUCCESS;
}

int handle(struct Client *client, struct Client *cli_base, struct Channel *channel_base) {
  int sockfd = client->fd;
  int cli_channel = client->id_channel;       //channel suscribed by the client to verify later on
  char alias[ALIAS_SIZE];
  strncpy(alias,client->alias,ALIAS_SIZE);
  char buffer_serv[BUFFER_SERV_SIZE];
  memset(buffer_serv, 0, BUFFER_SERV_SIZE);
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
  else { // extract the possible command of the msg receive, the argument and the data

    parse_request(buffer_serv, token_cmd, token_arg, token_data);

  }

  //Quit
  if(strcmp(token_cmd,QUIT_MSG) == 0) {
    printf("Quit msg received : connection closed by client\n");
    close(sockfd);
    return CLOSE_COMMUNICATION;
  }

  //Set nickname
  else if(strcmp(token_cmd,NICK_MSG) == 0) {

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
    broadcast(sockfd, cli_base, token_data);
    return KEEP_COMMUNICATION;
  }

  // Unicast
  else if(strcmp(token_cmd,UNICAST_MSG) == 0) {
    printf("Unicast msg received\n");
    unicast(sockfd, cli_base, token_data, token_arg);
    return KEEP_COMMUNICATION;
  }

  // Create channel
  else if(strcmp(token_cmd, CREATE_CHANNEL_MSG) == 0) {
    printf("Create channel msg received\n");
    create_channel(sockfd, token_arg, channel_base);
    return KEEP_COMMUNICATION;
  }

  // Join channel
  else if(strcmp(token_cmd, JOIN_MSG) == 0) {
    printf("Join channel msg received\n");
    join_channel(sockfd, cli_channel, token_arg, cli_base, channel_base);
    return KEEP_COMMUNICATION;
  }

  // Quit channel
  else if(strcmp(token_cmd, QUIT_CHANNEL_MSG) == 0) {
    printf("Quit channel msg received\n");
    quit_channel(sockfd, channel_base, client);
    return KEEP_COMMUNICATION;
  }

  // Upload demand
  else if (strcmp(token_cmd, UPLOAD_MSG) == 0){
    printf(" Send msg received\n");
    if ((token_arg == NULL)  || (token_data==NULL) ){
      // TODO informe_bad_request(sockfd_sender);
    }
    else{
      upload(sockfd, alias, token_arg, token_data, cli_base);
    }
    return KEEP_COMMUNICATION;
  }

  // Accept download
  else if (strcmp(token_cmd, ACCEPT_DOWNLOAD_MSG) == 0){
    printf(" Accept download msg received\n");
    accept_download(sockfd, alias, token_arg, cli_base);
    return KEEP_COMMUNICATION;
  }

  //Multicast
  else {
    printf("Multicast: sending to everyone in the channel\n");
    multicast(sockfd, alias, token_data, cli_channel, channel_base);
    printf("Done\n\n");

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
    cli_base[i].id_channel = NO_CHANNEL_YET;
  }
}

void init_channel_base(struct Channel *channel_base) {
  for(int i=0; i < MAX_NO_CHANNEL; i++) {
    //name
    channel_base[i].name[0] = '\0';

    //users by channel
    for (int j = 0; j < MAX_USERS_CHANNEL; j++) {
      channel_base[i].users_fd[j] = EMPTY_SLOT;
    }
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

  //security
  if (presence_alias(alias, cli_base) || strlen(alias) == 0) {
    printf("Nickname already used by another client or null alias\n");
    inform_nick_used(client->fd);
  }
  else {
    printf("Set nickname : %s\n", alias);
    strncpy(client->alias, alias, ALIAS_SIZE);
    inform_nick_set(client->fd, alias);
    printf("Nickname set\n");
  }

}

void inform_nick_used(int sockfd) {
  char buffer[BUFFER_CLI_SIZE];
  memset(buffer, 0, BUFFER_CLI_SIZE);
  strcpy(buffer, "[SERVER] Nickname already used by another client or incorrect nickname\n");

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
  strcat(buffer, " In the chat : \n");
  for (int i = 0; i < MAX_NO_CLI; i++) {
    if (cli_base[i].fd != EMPTY_SLOT) {
      strcat(buffer, "  - ");
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

void broadcast(int sockfd,struct Client *cli_base, char *msg){
  // Client buffer is higher than server buffer
  char* alias_sender = get_alias_from_fd(sockfd, cli_base);
  char buffer_cli[BUFFER_CLI_SIZE];
  memset(buffer_cli, 0, BUFFER_CLI_SIZE);

  snprintf(buffer_cli, BUFFER_CLI_SIZE, "[%s] msg all -> %s", alias_sender, msg);

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

}

void parse_request(char *buffer_serv, char *token_cmd, char *token_arg, char *token_data) {
  char buffer_serv_copy[BUFFER_SERV_SIZE];            // to extract token, we work only on a copy because strtok modifies the input
  memset(buffer_serv_copy, 0, BUFFER_SERV_SIZE);
  strncpy(buffer_serv_copy, buffer_serv, BUFFER_SERV_SIZE);   //to be clean when extracting tokens
  const char space[2] = " ";
  char* token = strtok(buffer_serv_copy, space);   //can be /quit /nick /whois /who  /msgall /msg

  //QUIT or WHO
  if ( (strcmp(token, QUIT_MSG) == 0) || (strcmp(token, WHO_MSG) == 0) ) {
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
      strncat(token_data, " ", BUFFER_CLI_SIZE);
    }

    return;
  }

  // UPLOAD MSG
  else if(strcmp(token, UPLOAD_MSG) == 0) {
    strncat(token_cmd, token, CMD_SIZE);  // /send
    if( (token=strtok(NULL, space)) != NULL)
      strncat(token_arg, token, ARG_SIZE); // <username>
    if( (token=strtok(NULL, space)) != NULL)
      strncat(token_data, token, ARG_SIZE);  //  "path/file.txt"
    return;
  }

  // ACCEPT DOWNLOAD MSG
  else if(strcmp(token, ACCEPT_DOWNLOAD_MSG) == 0) {
    strncat(token_cmd, token, CMD_SIZE);  // /accept
    if ( (token=strtok(NULL, space))!=NULL){
      for (int i=0; i<sizeof(token);i++){
        if (token[i] == '\n'){
          token[i]='\0';
        }
      }
      strncat(token_arg, token, ARG_SIZE); // <username>
    }
    return;
  }

  //UNICAST MSG
  else if (strcmp(token, UNICAST_MSG) == 0) {
    strncat(token_cmd, token, CMD_SIZE);
    strncat(token_arg, strtok(NULL, space), ARG_SIZE);

    // Walk through the data
    while( token != NULL ) {
      token = strtok(NULL, space);
      if (token != NULL) {strncat(token_data, token, BUFFER_CLI_SIZE);}
      strncat(token_data, " ", BUFFER_CLI_SIZE);
    }

    return;
  }

  //CREATE CHANNEL MSG
  else if(strcmp(token, CREATE_CHANNEL_MSG) == 0) {
    strncat(token_cmd, token, CMD_SIZE);
    strncat(token_arg, strtok(NULL, space), ARG_SIZE);

    return;
  }

  // QUIT CHANNEL MSG
  else if(strcmp(token, QUIT_CHANNEL_MSG) == 0) {
    strncat(token_cmd, token, CMD_SIZE);

    return;
  }

  //JOIN CHANNEL
  else if(strcmp(token, JOIN_MSG) == 0) {
    strncat(token_cmd, token, CMD_SIZE);
    strncat(token_arg, strtok(NULL, space), ARG_SIZE);

    return;
  }

  //MULTICAST
  else {
    if(token != NULL) {strncat(token_data, token, BUFFER_CLI_SIZE);}
    strncat(token_data, " ", BUFFER_CLI_SIZE);

    // Walk through the data
    while( token != NULL ){
      token = strtok(NULL, space);
      if (token != NULL) {strncat(token_data, token, BUFFER_CLI_SIZE);}
      strncat(token_data, " ", BUFFER_CLI_SIZE);
    }
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

char *get_ip_from_fd(int fd, struct Client *cli_base) {
  for(int i=0; i < MAX_NO_CLI; i++) {
    if(fd == cli_base[i].fd) {
      return cli_base[i].ip;
    }
  }
  return NULL;
}

void create_channel(int cli_fd, char *token_arg, struct Channel *channel_base) {

  //security
  if (presence_channel(token_arg, channel_base) || strlen(token_arg) == 0) {
    inform_channel_used(cli_fd);
  }
  else if(count_channels(channel_base) > MAX_NO_CHANNEL) {
    inform_channel_max(cli_fd);
  }

  else {
    printf("Create channel : %s\n", token_arg);
    for (int i = 0; i < MAX_NO_CHANNEL; i++) {
      if (strlen(channel_base[i].name) == 0) {
        strncpy(channel_base[i].name, token_arg, MAX_NAME_CHANNEL_SIZE);
        inform_channel_created(cli_fd);
        break;
      }
    }
  }

}

void inform_channel_used(int sockfd) {
  char buffer[BUFFER_CLI_SIZE];
  memset(buffer, 0, BUFFER_CLI_SIZE);
  strcpy(buffer, "[SERVER] Channel already used or incorrect name\n");

  do_send(sockfd, buffer, BUFFER_CLI_SIZE);
}

void inform_channel_created(int sockfd) {
  char buffer[BUFFER_CLI_SIZE];
  memset(buffer, 0, BUFFER_CLI_SIZE);
  strncpy(buffer, "[SERVER] Channel created\n", BUFFER_CLI_SIZE);

  do_send(sockfd, buffer, BUFFER_CLI_SIZE);
}

int presence_channel(char *channel_name, struct Channel *channel_base) {
  for(int i=0; i < MAX_NO_CHANNEL; i++) {
    if(strcmp(channel_name, channel_base[i].name) == 0) {
      return 1;
    }
  }
  return 0;
}

void join_channel(int cli_fd, int cli_channel, char *token_arg, struct Client *cli_base, struct Channel *channel_base) {

  int id_channel = get_id_channel_from_name(token_arg, channel_base);

  //security
  if (cli_channel != NO_CHANNEL_YET) {
    inform_inside_channel(cli_fd);
  }
  else if (!presence_channel(token_arg, channel_base)) {
    inform_channel_incorrect(cli_fd);
  }
  else if(count_users_channel(id_channel, channel_base) > MAX_USERS_CHANNEL) {
    inform_channel_crowded(cli_fd);
  }

  else {
    printf("%d joins channel : %s\n", cli_fd, token_arg);
    insert_client_channel(cli_fd, id_channel, channel_base);
    update_client_channel(id_channel, cli_fd, cli_base);      //updating the status of the user : associated channel
  }

}

void quit_channel(int sockfd,struct Channel *channel_base,struct Client* client) {
  int id_cli_channel=client->id_channel;

  // security
  if (id_cli_channel == -1) {
    inform_no_channel_yet(sockfd);
  }


  else if( count_users_channel(id_cli_channel, channel_base) == 1){
    destroy_channel(channel_base+id_cli_channel);  // destroy the channel if it's the last user
    client->id_channel=-1;
    inform_quit_success(sockfd);
    }
  else{
    remove_cli_from_channel(channel_base+id_cli_channel,sockfd); // Quit the channel
    client->id_channel=-1;
    inform_quit_success(sockfd);

  }
}

void insert_client_channel(int cli_fd, int id_channel, struct Channel *channel_base) {

  int *users_fd = channel_base[id_channel].users_fd;    //considering the users database of the channel to modify
  for (int i = 0; i < MAX_USERS_CHANNEL; i++) {
    if (users_fd[i] == EMPTY_SLOT) {
      users_fd[i] = cli_fd;
      inform_channel_joined(cli_fd);
      break;
    }
  }
}

void update_client_channel(int id_channel, int cli_fd, struct Client *cli_base) {
  for (int i = 0; i < MAX_NO_CLI; i++) {
    if (cli_base[i].fd == cli_fd) {
      cli_base[i].id_channel = id_channel;
      break;
    }
  }
}

void inform_no_channel_yet(int sockfd) {
  char buffer[BUFFER_CLI_SIZE];
  memset(buffer, 0, BUFFER_CLI_SIZE);
  strncpy(buffer, "[SERVER] You are not actually in a channel\n",BUFFER_CLI_SIZE);

  do_send(sockfd, buffer, BUFFER_CLI_SIZE);
}

void inform_quit_success(int sockfd) {
  char buffer[BUFFER_CLI_SIZE];
  memset(buffer, 0, BUFFER_CLI_SIZE);
  strncpy(buffer, "[SERVER] You have quit the channel with success\n",BUFFER_CLI_SIZE);

  do_send(sockfd, buffer, BUFFER_CLI_SIZE);
}

void destroy_channel(struct Channel *channel) {
    channel->name[0]='\0';
    for (int i = 0; i < MAX_USERS_CHANNEL; i++) {
      channel->users_fd[i]=EMPTY_SLOT;
    }
}

void remove_cli_from_channel(struct Channel * channel, int sockfd) {
  for (int i = 0; i < MAX_USERS_CHANNEL; i++) {
    if (channel->users_fd[i]==sockfd) {
      channel->users_fd[i]=EMPTY_SLOT;
    }
  }
}

int get_id_channel_from_name(char *name, struct Channel *channel_base) {
  for (int i = 0; i < MAX_NO_CLI; i++) {
    if ((strcmp(channel_base[i].name, name) == 0)) {
      return i;     //our design : id channel is considered as index of channel database
    }
  }

  return NO_CHANNEL_YET;
}

void multicast(int sockfd,char *alias_sender, char*msg, int id_channel, struct Channel *channel_base){
  char buffer_cli[BUFFER_CLI_SIZE];
  memset(buffer_cli, 0, BUFFER_CLI_SIZE);

  // security
  if (id_channel==NO_CHANNEL_YET){
    inform_join_channel(sockfd);
  }
  else{
    snprintf(buffer_cli, BUFFER_CLI_SIZE, "[%s] -> %s", alias_sender, msg);
    for (int i = 0; i < MAX_USERS_CHANNEL; i++) {
      if(channel_base[id_channel].users_fd[i] != sockfd && channel_base[id_channel].users_fd[i] != EMPTY_SLOT){
        do_send(channel_base[id_channel].users_fd[i], buffer_cli, BUFFER_CLI_SIZE);
      }
    }
  }
}

void inform_channel_crowded(int sockfd) {
  char buffer[BUFFER_CLI_SIZE];
  memset(buffer, 0, BUFFER_CLI_SIZE);
  strncpy(buffer, "[SERVER] Channel crowded, please try again later\n", BUFFER_CLI_SIZE);

  do_send(sockfd, buffer, BUFFER_CLI_SIZE);
}

void inform_channel_incorrect(int sockfd) {
  char buffer[BUFFER_CLI_SIZE];
  memset(buffer, 0, BUFFER_CLI_SIZE);
  strncpy(buffer, "[SERVER] Channel incorrect, not present or spelled wrong\n", BUFFER_CLI_SIZE);

  do_send(sockfd, buffer, BUFFER_CLI_SIZE);
}

void inform_channel_max(int sockfd) {
  char buffer[BUFFER_CLI_SIZE];
  memset(buffer, 0, BUFFER_CLI_SIZE);
  strncpy(buffer, "[SERVER] Too many channels already created, please come back later\n", BUFFER_CLI_SIZE);

  do_send(sockfd, buffer, BUFFER_CLI_SIZE);
}

void inform_channel_joined(int sockfd) {
  char buffer[BUFFER_CLI_SIZE];
  memset(buffer, 0, BUFFER_CLI_SIZE);
  strncpy(buffer, "[SERVER] Channel joined\n", BUFFER_CLI_SIZE);

  do_send(sockfd, buffer, BUFFER_CLI_SIZE);
}

void inform_inside_channel(int sockfd) {
  char buffer[BUFFER_CLI_SIZE];
  memset(buffer, 0, BUFFER_CLI_SIZE);
  strncpy(buffer, "[SERVER] You are already in a channel, please leave first with /quit\n", BUFFER_CLI_SIZE);

  do_send(sockfd, buffer, BUFFER_CLI_SIZE);
}

int count_channels(struct Channel *channel_base) {
  int no_channels = 0;
  for (int i = 0; i < MAX_NO_CHANNEL; i++) {
    if (strlen(channel_base[i].name) > 0) {
      no_channels++;
    }
  }
}

int count_users_channel(int id_channel, struct Channel *channel_base) {
  int no_users = 0;
  for (int i = 0; i < MAX_USERS_CHANNEL; i++) {
    if (channel_base[id_channel].users_fd[i] != EMPTY_SLOT) {
      no_users++;
    }
  }
}

void inform_join_channel(int sockfd) {
  char buffer[BUFFER_CLI_SIZE];
  memset(buffer, 0, BUFFER_CLI_SIZE);
  strncpy(buffer, "[SERVER] You need to join a channel with the /join or create your own with /create <ChannelName>\n",BUFFER_CLI_SIZE);

  do_send(sockfd, buffer, BUFFER_CLI_SIZE);
}

void clean_channel(struct Client* client, struct Channel *channel_base) {
  int sockfd = client->fd;
  int id_cli_channel=client->id_channel;

  // security for when client hasn't suscribed to a channel
  if (id_cli_channel == NO_CHANNEL_YET) {
    printf("OK client wasn't in a channel\n");
  }

  else if( count_users_channel(id_cli_channel, channel_base) == 1){
    destroy_channel(channel_base+id_cli_channel);  // destroy the channel if it's the last user
    }
  else{
    remove_cli_from_channel(channel_base+id_cli_channel, sockfd); // Quit the channel
  }
}


void upload(int sockfd_sender,char* alias_sender, char* alias_receiver, char* filename, struct Client * cli_base) {

  //security

  if ( (presence_alias(alias_receiver, cli_base)==0) || (strcmp(alias_receiver, alias_sender)==0)) {
    inform_alias_incorrect(sockfd_sender);
  }
  else {
    char buffer_cli[BUFFER_CLI_SIZE];
    memset(buffer_cli, 0, BUFFER_CLI_SIZE);

    int sockfd_receiver=get_fd_from_alias(alias_receiver, cli_base);
    snprintf(buffer_cli, BUFFER_CLI_SIZE, "{DOWNLOAD} %s wants you to accept the transfer of the file named \"%s\".\n You can accept with the command /accept %s \n",alias_sender,filename,alias_sender);
    do_send(sockfd_receiver, buffer_cli, BUFFER_CLI_SIZE);

    memset(buffer_cli, 0, BUFFER_CLI_SIZE);
    strncpy(buffer_cli, WAIT_ACCEPT_MSG,BUFFER_CLI_SIZE);
    do_send(sockfd_sender, buffer_cli, BUFFER_CLI_SIZE);
  }
}


/*void accept_download_demand(int sockfd_sender, char*alias_sender, char*alias_receiver, char*filename, struct Client * cli_base){
  char buffer_cli[BUFFER_CLI_SIZE];
  memset(buffer_cli, 0, BUFFER_CLI_SIZE);

  int sockfd_receiver=get_fd_from_alias(alias_receiver, cli_base);
  snprintf(buffer_cli, BUFFER_CLI_SIZE, "%s %s wants you to accept the transfer of the file named \"%s\".\n You can accept with the command /accept %s \n", DOWNLOAD_FLAG,alias_sender,filename,alias_sender);
  do_send(sockfd_receiver, buffer_cli, BUFFER_CLI_SIZE);

  memset(buffer_cli, 0, BUFFER_CLI_SIZE);
  strncpy(buffer_cli, WAIT_ACCEPT_MSG,BUFFER_CLI_SIZE);
  do_send(sockfd_sender, buffer_cli, BUFFER_CLI_SIZE);
}*/

void accept_download(int sockfd_sender, char * alias_sender, char* alias_receiver,struct Client * cli_base) {
  char *ip_sender;
  char *ip_receiver;
  char buffer_cli[BUFFER_CLI_SIZE];
  memset(buffer_cli, 0, BUFFER_CLI_SIZE);
  int sockfd_receiver;

  // security
  if ( presence_alias(alias_receiver, cli_base) ==0 || (strcmp(alias_receiver, alias_sender)==0)) {
      inform_alias_incorrect(sockfd_sender);
  }
  else {
    if ( alias_receiver[ sizeof(alias_receiver)-1 ] == '\n' ) {
      alias_receiver[ sizeof(alias_receiver) -1 ] = '\0' ;
    }
    sockfd_receiver=get_fd_from_alias(alias_receiver, cli_base);
    ip_sender=get_ip_from_fd(sockfd_sender, cli_base);
    snprintf(buffer_cli, BUFFER_CLI_SIZE, "%s %s %s", UPLOAD_FLAG,alias_sender,ip_sender);
    do_send(sockfd_receiver, buffer_cli, BUFFER_CLI_SIZE);

    memset(buffer_cli, 0, BUFFER_CLI_SIZE);
    strncpy(buffer_cli, WAIT_DEMAND_MSG,BUFFER_CLI_SIZE);
    do_send(sockfd_sender, buffer_cli, BUFFER_CLI_SIZE);

    memset(buffer_cli, 0, BUFFER_CLI_SIZE);
    ip_receiver=get_ip_from_fd(sockfd_receiver, cli_base);
    snprintf(buffer_cli, BUFFER_CLI_SIZE, "%s %s %s", DOWNLOAD_FLAG,alias_receiver,ip_receiver);
    do_send(sockfd_sender, buffer_cli, BUFFER_CLI_SIZE);

  }
}
