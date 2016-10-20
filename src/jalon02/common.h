#ifndef COMMON_H
#define COMMON_H

#define WELCOME_MSG "Welcome! Connection established with the Talkr server\n"
#define REFUSE_MSG "Refused! Server cannot accept anymore incoming connections, retry later...\n"

void error(const char *msg);
int create_socket();

#endif
