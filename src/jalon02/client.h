#define h_addr h_addr_list[0]   /* for backward compatibility */
#define BUFFER_SIZE 256
#define QUIT_MSG "/quit\n"

void error(const char *msg);
int create_socket();
struct hostent* get_server(const char *host_target);
void init_serv_address(struct hostent* server, struct sockaddr_in* serv_addr_ptr, int port_no);
