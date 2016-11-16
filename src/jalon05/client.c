#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h> // struct sockaddr_in
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include "common.h"
#include "client.h"


int main(int argc, char const *argv[]) {
  char alias[ALIAS_SIZE];
  char buffer_send[BUFFER_SERV_SIZE];
  char buffer_receive[BUFFER_CLI_SIZE];
  // for server
  int srv_sockfd;
  int port_no;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  // for download
  struct Download download;
  int sockfd_download;
  int sockfd_upload;
  struct sockaddr_in uploader_addr, downloader_addr;
  // for select
  fd_set read_fds, read_fds_copy;   //copy because of select, to be clean
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  off_t offset=0;



  //Verifying arguments
  if (argc < 3) {
    fprintf(stderr,"Program %s needs arguments regarding target server: hostname, port\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  port_no = atoi(argv[2]);

/*   SERVER - CLIENT */
  //Preparing
  srv_sockfd = create_socket();
  server = get_server(argv[1]);
  init_serv_address(server, &serv_addr, port_no);
  strcpy(alias,"NO_NICKNAME_SET"); // set nickname
  init_download(&download);

  //Connect to server
  if ( connect(srv_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))<0 ) {
    error("Error - connection");
  }

  //Receive Welcome/Refuse msg
  memset(buffer_receive, 0, BUFFER_CLI_SIZE);
  do_recv(srv_sockfd, buffer_receive, BUFFER_CLI_SIZE);
  printf("%s\n", buffer_receive);
  if (strcmp(buffer_receive, REFUSE_MSG) == 0){
    error("Error - connection");
  }

  //  Welcome msg
  printf("\n-------------------------\n-    Talkr Client !    -\n-------------------------\n");
  printf("Write '/quit' in order to close this session\n\n\n");





  //Main Client Loop
  while(1) {

    //Workspace fd set preparation
    FD_ZERO(&read_fds);
    int max_fd=srv_sockfd+1;
    FD_SET(0, &read_fds);
    FD_SET(srv_sockfd, &read_fds);


    //Activity monitoring
    read_fds_copy = read_fds;
    if(select(max_fd + 1, &read_fds_copy, NULL, NULL, &tv) == -1) {
      error("Error - select");
    }

    // Msg from server
    if(FD_ISSET(srv_sockfd, &read_fds_copy)){
      //Receive Echo
      memset(buffer_receive, 0, BUFFER_CLI_SIZE);
      do_recv(srv_sockfd, buffer_receive, BUFFER_CLI_SIZE);
      printf("Msg received: %s\n", buffer_receive);
      //printf("%s\n%s\n....", buffer_receive,WAIT_ACCEPT_MSG);

      if (strncmp(buffer_receive,ALIAS_MSG,sizeof(ALIAS_MSG)-1)==0) { // the client nickname is save localy
        set_alias(buffer_receive,alias);
      }

      // wait for Acceptance
      if (strncmp(buffer_receive, WAIT_ACCEPT_MSG,sizeof(WAIT_ACCEPT_MSG)-1) == 0) {
        download.cli_state=WAIT_ACCEPT;
        //Preparing client
        sockfd_upload = create_socket();
        init_uploader_address(&uploader_addr, DOWNLOAD_PORT);
        do_bind(sockfd_upload, &uploader_addr);

        //Listen
        if(listen(sockfd_upload, 1) < 0) {
          error("Error - listen");
        }
        printf("Listening ...\n");
      }
      // init upload
      if (strncmp(buffer_receive, UPLOAD_FLAG,sizeof(UPLOAD_FLAG)-1) == 0) {
        download.cli_state=UPLOAD_IN_PROGRESS;
        printf("Init upload in progress ok\n");

      }
      // init Download
      if (strncmp(buffer_receive, DOWNLOAD_FLAG,sizeof(DOWNLOAD_FLAG)-1) == 0) {
        char ip_uploader[IP_SIZE] = {'\0'};
        struct hostent* cli_uploader;
        struct sockaddr_in uploader_addr;
        download.cli_state=DOWNLOAD_IN_PROGRESS;

        //Preparing
        sockfd_download = create_socket();
        extract_ip(buffer_receive, ip_uploader);
        printf("extract is %s\n", ip_uploader);
        cli_uploader = get_server(ip_uploader);
        init_serv_address(cli_uploader, &uploader_addr, DOWNLOAD_PORT);

        //Connect to server
        if ( connect(sockfd_download, (struct sockaddr *) &uploader_addr, sizeof(uploader_addr))<0 ) {
          error("Error - connection");
        }

        printf("Connected.\n");
      }
    }

    // Input message
    if(FD_ISSET(0, &read_fds_copy)) {
      memset(buffer_send, 0, BUFFER_SERV_SIZE);
      fgets(buffer_send, BUFFER_SERV_SIZE, stdin);


      //Quit
      if (strcmp(buffer_send, QUIT_MSG) == 0) {
        printf("Client decided to quit the chat\n");
        close(srv_sockfd);
        return EXIT_SUCCESS;
      }
      // download restriction : only one at the same time
      if ( strncmp(buffer_send, UPLOAD_MSG, sizeof(UPLOAD_MSG)-1) == 0  &&  download.cli_state != NO_ACTIVITY ) {
          printf("You have already a current connection with an other client. Try later on \n");
      }
      else if (sizeof(buffer_send)!=1) { // prevent empty msg
        printf("MSG send: %s \n",buffer_send);
        update_download(buffer_send,&download);
        do_send(srv_sockfd, buffer_send, BUFFER_SERV_SIZE);
      }
    }

    // download
    if(FD_ISSET(sockfd_download, &read_fds_copy) && download.cli_state == DOWNLOAD_IN_PROGRESS) {
      printf("Downloader is in the if\n");
      memset(buffer_receive, 0, BUFFER_CLI_SIZE);
      int filefd=open("downloadfile", O_WRONLY | O_APPEND | O_CREAT, S_IRWXU);
      if (do_recv(sockfd_download, buffer_receive, BUFFER_CLI_SIZE)==0){
          close(filefd);
          close(sockfd_download);
          download.cli_state=NO_ACTIVITY;
      }
      write(filefd,buffer_receive,BUFFER_CLI_SIZE);
      close(filefd);

    }

    if(download.cli_state == UPLOAD_IN_PROGRESS) {
      printf("I'm in\n");
      memset(buffer_receive, 0, BUFFER_CLI_SIZE);
      int filefd=open(download.file, O_RDONLY | O_CREAT, S_IRWXU);
      printf("After open\n");

      if( (pread(filefd,buffer_receive,BUFFER_CLI_SIZE,offset) == 0)) {
        printf("In if\n");
        close(filefd);
        close(sockfd_upload);
        download.cli_state=NO_ACTIVITY;
      }
      printf("%s sur fd=%i\n", buffer_receive,sockfd_upload);

      //if (send(sockfd_upload, buffer_receive, BUFFER_CLI_SIZE, 0) == -1) {
      //  printf("Error send");
      //}
      do_send(sockfd_upload, buffer_receive, BUFFER_CLI_SIZE);
      printf("%s\n","SEND ok" );
      close(filefd);
      offset+=BUFFER_CLI_SIZE;
      printf("I'm out\n");
    }




  }// end while
  return EXIT_SUCCESS;  //optional line, indeed client quits the program with "/quit" or ctrl+c
}




/*

  Functions

*/


struct hostent* get_server(const char *host_target) {
  struct hostent *server = gethostbyname(host_target);	//Later on: use addrinfo (cf. gethostbyname considered deprecated, and for ipv6...etc)
  if (server == NULL) {
    fprintf(stderr, "Error: No such host\n");
    exit(EXIT_FAILURE);
  }

  return server;
}

void init_serv_address(struct hostent* server, struct sockaddr_in* serv_addr_ptr, int port_no) {
  memset(serv_addr_ptr, 0, sizeof(struct sockaddr_in));
  serv_addr_ptr->sin_family = AF_INET;
  memcpy(server->h_addr, &(serv_addr_ptr->sin_addr.s_addr), server->h_length);
  serv_addr_ptr->sin_port = htons(port_no);  //convert to network order
}

void set_alias(char *buffer,char *alias) {
  int i=0;
  const char arrow[3] = "->";
  char *token;

  token = strtok(buffer, arrow);
  token = strtok(NULL, arrow);
  strcpy(alias, token);
  while (alias[i]!='\n') {
    i++;
  }
  alias[i-1]='\0';
  alias[sizeof(alias)-1] = '\0';
}

void init_download(struct Download *download) {
  download->cli_state=NO_ACTIVITY;
  download->alias[0]='\0';
  download->file[0]='\0';
  download->ip=-1;
  //download->port=0;
}

void update_download(char *buffer_send,struct Download *download) {

  if ( strncmp(buffer_send, UPLOAD_MSG, sizeof(UPLOAD_MSG)-1) == 0) {

    char buffer_send_copy[BUFFER_SERV_SIZE];            // to extract token, we work only on a copy because strtok modifies the input
    memset(buffer_send_copy, 0, BUFFER_SERV_SIZE);
    strncpy(buffer_send_copy, buffer_send, BUFFER_SERV_SIZE);
    const char space[2] = " ";
    char* token_cmd = strtok(buffer_send_copy, space);
    char* token_nickname= strtok(NULL, space);
    char* token_file= strtok(NULL, space);

    // security TODO check if the file exist and return

    // Update of download structure
    if ( token_nickname != NULL ) {
      strcpy(download->alias, token_nickname);
    }
    if (token_file != NULL) {
      token_file[strlen(token_file)-1] = '\0';
      strcpy(download->file, token_file);
    }
  }
}

void init_uploader_address(struct sockaddr_in *serv_addr_ptr, int port_no) {
  memset(serv_addr_ptr, 0, sizeof(struct sockaddr_in));
  serv_addr_ptr->sin_family = AF_INET;
  serv_addr_ptr->sin_addr.s_addr = htonl(INADDR_ANY);  //INADDR_ANY : all interfaces - not just "localhost", multiple network interfaces OK
  serv_addr_ptr->sin_port = htons(port_no);  //convert to network order
}

void do_bind(int sockfd, struct sockaddr_in *serv_addr_ptr) {
  if ( bind(sockfd, (struct sockaddr *) serv_addr_ptr, sizeof(struct sockaddr_in))<0 ) {  //cast generic struct
    error("Error - bind");
  }
}

void extract_ip(char *buffer_receive, char *ip_uploader) {
  const char space[2] = " ";
  char* token = strtok(buffer_receive, space);   //can be /quit /nick /whois /who  /msgall /msg
  token = strtok(NULL, space);
  token = strtok(NULL, space);

  strncat(ip_uploader, token, IP_SIZE);
}
