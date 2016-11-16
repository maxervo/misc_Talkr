#ifndef CLIENT_H
#define CLIENT_H

#define h_addr h_addr_list[0]   /* for backward compatibility */

enum cli_state {
	NO_ACTIVITY=1,  // no file to receive or send
	WAIT_ACCEPT,
	DOWNLOAD_IN_PROGRESS,
	UPLOAD_IN_PROGRESS
};

struct Download {
  enum cli_state cli_state;
  char alias[ALIAS_SIZE];
  char file[FILEPATH_SIZE];
  int ip;
  //int port; We considered the port no NUM_PORT as the default port (for download OR upload)
};


struct hostent* get_server(const char *host_target);
void init_serv_address(struct hostent* server, struct sockaddr_in* serv_addr_ptr, int port_no);
void set_alias(char *buffer,char *alias);
void init_download(struct Download *download);
void update_download(char *buffer_send,struct Download *download);


void init_uploader_address(struct sockaddr_in *serv_addr_ptr, int port_no);
void do_bind(int sockfd, struct sockaddr_in *serv_addr_ptr); // TODO put in common.c

void extract_ip(char *buffer_receive, char *ip_uploader);

#endif
