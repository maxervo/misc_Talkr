#ifndef COMMON_H
#define COMMON_H

#define REFUSE_MSG "[SERVER] Refused! Server cannot accept anymore incoming connections, retry later...\n"
#define WELCOME_MSG "[SERVER] Welcome! Connection established with the Talkr server\nFirt of all, you need a nickname : use the commande /nick <YourNickname>"
#define ALIAS_MSG "[SERVER] new nickname -> "

#define ALIAS_SIZE 25
#define ROOM_SIZE 25
#define IP_SIZE 25
#define TOLERANCE 25
#define CMD_SIZE 25
#define ARG_SIZE ALIAS_SIZE+TOLERANCE //adding tolerance
#define MAX_NO_CLI 20
#define BUFFER_SERV_SIZE 500     //Adapt according to the size of alias and max no client
#define BUFFER_CLI_SIZE BUFFER_SERV_SIZE+ALIAS_SIZE+TOLERANCE     // adding a tolerance    //TODO: verify if bloc ok when preprocessor inserts it, need maybe bloc or ()
#define QUIT_MSG "/quit\n"  //Attention newline character captured as well by input
#define WHO_MSG "/who\n"
#define NICK_MSG "/nick"
#define WHOIS_MSG "/whois"
#define BROADCAST_MSG "/msgall"
#define UNICAST_MSG "/msg"
#define CLOSE_ABRUPT -1

void error(const char *msg);
int create_socket();
void do_send(int sockfd, char *buffer, int buffer_size);
int do_recv(int sockfd, char *buffer, int buffer_size);

#endif
