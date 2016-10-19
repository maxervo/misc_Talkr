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
int do_socket();
void init_serv_address(struct sockaddr_in *serv_addr_ptr, int port_no);
void do_bind(int sockfd, struct sockaddr_in *serv_addr_ptr);
int slotfd_available(struct client cli_base[]);
int welcome(int sockfd);
int refuse(int sockfd);
void init_client_base(struct client *cli_base);

void error(const char *msg)   //ATTENTION : program flow exit
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int handle(int sockfd) {
  char buffer[BUFFER_SIZE];
  memset(buffer, 0, BUFFER_SIZE);
  int readlen = recv(sockfd, buffer, BUFFER_SIZE, 0);     //Later on: add a security "do while" loop for bytes, interesting for busy interface or embedded systems with small network buffer

  //Quit
  if (readlen == 0 || strcmp(buffer,QUIT_MSG) == 0) { //other than quit msg, recv returns 0 when client closes connection as well
    printf("Connection closed by client\n");
    return CLOSE_COMMUNICATION;
  }

  //Error
  else if (readlen < 0) {
    error("Error - reception");
  }

  //Msg
  else {
    printf("[%i]: %s",sockfd, buffer);

    //Echo
    if (send(sockfd, buffer, BUFFER_SIZE, 0) < 0) {       //Later on: add a security "do while" loop for bytes, interesting for busy interface or embedded systems with small network buffer
      error("Error - echo emission");
    }
    printf("Echo sent to [%i]\n----------------------------\n----------------------------\n", sockfd);

    return KEEP_COMMUNICATION;
  }

}

int do_socket() {
  int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);   //Sockets config: Blocking  //Possible to add SO_REUSEADDR with setsockopt() during dev phase testing...etc
  if (sockfd < 0) {
    error("Error - socket opening");
  }

  return sockfd;
}

void init_serv_address(struct sockaddr_in *serv_addr_ptr, int port_no) {
  memset(serv_addr_ptr, 0, sizeof(struct sockaddr_in));
  serv_addr_ptr->sin_family = AF_INET;
  serv_addr_ptr->sin_addr.s_addr = htonl(INADDR_ANY);  //INADDR_ANY : all interfaces - not just "localhost", multiple network interfaces OK
  serv_addr_ptr->sin_port = htons(port_no);  //convert to network order
}

void do_bind(int sockfd, struct sockaddr_in *serv_addr_ptr) {
  if ( bind(sockfd, (struct sockaddr *) serv_addr_ptr, sizeof(struct sockaddr_in))<0 ) {  //need cast generic
    error("Error - bind");
  }
}

int slotfd_available(struct client cli_base[]) {
  int i = 0;
  while(i <= MAX_NO_CLI) {
    if (cli_base[i].fd == 0) {
      return i;
    }
    else {
      i++;
    }
  }
  return SLOTFD_UNAVAILABLE;
}

int welcome(int sockfd) {
  char buffer[BUFFER_SIZE];
  memset(buffer, 0, BUFFER_SIZE);
  char * welcome="-------------------------\n- Welcome to the chat ! -\n-------------------------\n write '/quit' in order to close this session\n\n Input msg:\n ";

  strcpy(buffer,welcome);

  if (send(sockfd, buffer, BUFFER_SIZE, 0) < 0) {       //Later on: add a security "do while" loop for bytes, interesting for busy interface or embedded systems with small network buffer
    error("Error - welcome emission");
  }
  printf("----------------------------\n------New Connection !------\n----------------------------\n");

  return KEEP_COMMUNICATION;
}

int refuse(int sockfd) {
  char buffer[BUFFER_SIZE];
  memset(buffer, 0, BUFFER_SIZE);
  char * msg="\n-------------------------\n--  Retry again later ! --\n-------------------------\n (Server cannot accept incoming connections anymore)\n\n";

  strcpy(buffer,msg);

  if (send(sockfd, buffer, BUFFER_SIZE, 0) < 0) {       //Later on: add a security "do while" loop for bytes, interesting for busy interface or embedded systems with small network buffer
    error("Error - welcome emission");

  }
  printf("----------------------------\n--New connection refused !--\n----------------------------\n");
  return KEEP_COMMUNICATION;
}

void init_client_base(struct client *cli_base) {
  memset(cli_base, 0, MAX_NO_CLI*sizeof(struct client));  //Considered NULL values 0, clean way would be manual init
}
