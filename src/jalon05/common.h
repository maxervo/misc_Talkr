#ifndef COMMON_H
#define COMMON_H

#define REFUSE_MSG "[SERVER] Refused! Server cannot accept anymore incoming connections, retry later...\n"
#define WELCOME_MSG "[SERVER] Welcome! Connection established with the Talkr server\nFirt of all, you need a nickname : use the commande /nick <YourNickname>"
#define ALIAS_MSG "[SERVER] new nickname -> "
#define WAIT_ACCEPT_MSG "[SERVER] Ok, your demand has been sent. Waiting for Acceptance. \n"
#define WAIT_DEMAND_MSG "[SERVER] Your demand has been sent. \n"

#define ALIAS_SIZE 25
#define ROOM_SIZE 25
#define IP_SIZE 25
#define TOLERANCE 25
#define CMD_SIZE 25
#define ARG_SIZE ALIAS_SIZE+TOLERANCE //adding tolerance
#define MAX_NO_CLI 20
#define BUFFER_SERV_SIZE 500     //Adapt according to the size of alias and max no client
#define BUFFER_CLI_SIZE BUFFER_SERV_SIZE+ALIAS_SIZE+TOLERANCE     // adding a tolerance
#define FILEPATH_SIZE 100
#define QUIT_MSG "/quit\n"  //Attention newline character captured as well by input
#define WHO_MSG "/who\n"
#define NICK_MSG "/nick"
#define WHOIS_MSG "/whois"
#define BROADCAST_MSG "/msgall"
#define UNICAST_MSG "/msg"
#define CREATE_CHANNEL_MSG "/create"
#define QUIT_CHANNEL_MSG "/quitchannel\n"
#define JOIN_MSG "/join"
#define UPLOAD_MSG "/send"
#define ACCEPT_DOWNLOAD_MSG "/accept"
#define DOWNLOAD_FLAG "{DOWNLOAD}"
#define UPLOAD_FLAG "{UPLOAD}"
#define START_DOWNLOAD_FLAG "{START_DOWNLOAD}"

#define DOWNLOAD_PORT 2555

#define CLOSE_ABRUPT -1

void error(const char *msg);
int create_socket();
void do_send(int sockfd, char *buffer, int buffer_size);
int do_recv(int sockfd, char *buffer, int buffer_size);


#endif
