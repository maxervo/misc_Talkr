#ifndef SERVER_H
#define SERVER_H

#define MAX_NO_CLI 20
#define MAX_NUM_QUEUE 10
#define ALIAS_SIZE 25
#define CON_TIME_SIZE 25

#define SLOT_UNAVAILABLE -1
#define KEEP_COMMUNICATION 1
#define CLOSE_COMMUNICATION 0
#define EMPTY_SLOT 0


struct Client {
  int fd;
  char alias[ALIAS_SIZE];
  char con_time[CON_TIME_SIZE];
  long ip_addr;  //Format with inet_aton(), inet_ntoa()
};

void error(const char *msg);
int handle(struct Client *cli_base);
void init_serv_address(struct sockaddr_in *serv_addr_ptr, int port_no);
void do_bind(int sockfd, struct sockaddr_in *serv_addr_ptr);
int slot_available(struct Client cli_base[]);
void welcome(int sockfd);
void refuse(int sockfd);
void init_client_base(struct Client *cli_base);
void set_nickname(struct Client *cli_base, char *alias);




#endif
