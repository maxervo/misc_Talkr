//#define MAX_NO_CONNECTIONS 20
#define MAX_NO_CLI 2
#define MAX_NUM_QUEUE 10
#define BUFFER_SIZE 256
#define ALIAS_SIZE 25
#define CON_TIME_SIZE 25
#define QUIT_MSG "/quit\n"  //Attention newline character captured as well by input
#define SLOTFD_UNAVAILABLE -1

#define KEEP_COMMUNICATION 1
#define CLOSE_COMMUNICATION 0

struct client {
  int fd;
  char alias[ALIAS_SIZE];
  char con_time[CON_TIME_SIZE];
  long ip_addr;  //ATTENTION with inet_aton(), inet_ntoa()
};

void error(const char *msg);
int handle(int sockfd);
int create_socket();
void init_serv_address(struct sockaddr_in *serv_addr_ptr, int port_no);
void do_bind(int sockfd, struct sockaddr_in *serv_addr_ptr);
int slotfd_available(struct client cli_base[]);
int welcome(int sockfd);
int refuse(int sockfd);
void init_client_base(struct client *cli_base);
