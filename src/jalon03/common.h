#ifndef COMMON_H
#define COMMON_H

#define REFUSE_MSG "Refused! Server cannot accept anymore incoming connections, retry later...\n"
#define WELCOME_MSG "Welcome! Connection established with the Talkr server\nFirt of all, you need a nickname : use the commande /nick\n(example : >> /nick MyNickname)\n"

#define ALIAS_SIZE 25
#define IP_SIZE 25
#define MAX_NO_CLI 20
#define BUFFER_SIZE 500     //Adapt according to the size of alias and max no client
#define QUIT_MSG "/quit\n"  //Attention newline character captured as well by input
#define WHO_MSG "/who\n"
#define NICK_MSG "/nick"
#define WHOIS_MSG "/whois"
#define CLOSE_ABRUPT -1

void error(const char *msg);
int create_socket();
void do_send(int sockfd, char *buffer, int buffer_size);
int do_recv(int sockfd, char *buffer, int buffer_size);

#endif
