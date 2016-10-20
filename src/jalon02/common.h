#ifndef COMMON_H
#define COMMON_H

#define WELCOME_MSG "Welcome! Connection established with the Talkr server\n"
#define REFUSE_MSG "Refused! Server cannot accept anymore incoming connections, retry later...\n"

#define BUFFER_SIZE 256
#define QUIT_MSG "/quit\n"  //Attention newline character captured as well by input
#define CLOSE_ABRUPT -1

void error(const char *msg);
int create_socket();
void do_send(int sockfd, char *buffer, int buffer_size);
int do_recv(int sockfd, char *buffer, int buffer_size);

#endif
