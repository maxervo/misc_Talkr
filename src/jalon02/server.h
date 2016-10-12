#define MAX_NO_CONNECTIONS 20
#define MAX_NUM_QUEUE 10
#define BUFFER_SIZE 256
#define QUIT_MSG "/quit\n"  //Attention newline character captured as well by input

#define KEEP_COMMUNICATION 1
#define CLOSE_COMMUNICATION 0

void error(const char *msg);
int handle(int sockfd);
int do_socket();
void init_serv_address(struct sockaddr_in *serv_addr_ptr, int port_no);
void do_bind(int sockfd, struct sockaddr_in *serv_addr_ptr);
