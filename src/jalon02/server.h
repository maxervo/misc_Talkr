#ifndef SERVER_H
#define SERVER_H

#define MAX_NUM_QUEUE 10
#define CON_TIME_SIZE 25

#define SLOT_UNAVAILABLE -1
#define KEEP_COMMUNICATION 1
#define CLOSE_COMMUNICATION 0
#define EMPTY_SLOT 0

struct Client {
  int fd;
  char alias[ALIAS_SIZE];
  char con_time[CON_TIME_SIZE];
  char ip[IP_SIZE];  //Format with inet_aton(), inet_ntoa()
  int port;
};

void error(const char *msg);
int handle(struct Client *client, struct Client *cli_base);
void init_serv_address(struct sockaddr_in *serv_addr_ptr, int port_no);
void do_bind(int sockfd, struct sockaddr_in *serv_addr_ptr);
int slot_available(struct Client cli_base[]);
void welcome(int sockfd);
void refuse(int sockfd);
void init_client_base(struct Client *cli_base);
void set_nickname(struct Client *client, char *alias, struct Client *cli_base);
void inform_nick_set(int sockfd);
void inform_nick_demand(int sockfd);
int check_nickname(char *alias, struct Client *cli_base);
void inform_nick_used(int sockfd);
void reset_client_slot(struct Client *client);
void inform_who(int sockfd,struct Client *cli_base);
void set_client(struct Client* client, int new_sockfd, struct sockaddr_in cli_addr);
void inform_whois(struct Client *client, char *alias, struct Client *cli_base);

#endif
