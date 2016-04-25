#include <errno.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>  // htons() and inet_addr()
#include <netinet/in.h> // struct sockaddr_in
#include <sys/socket.h>

#include "common.h"

int main(int argc, char* argv[]) {
    int ret;

    // variables for handling a socket
    int socket_desc;
    struct sockaddr_in server_addr = {0}; // some fields are required to be filled with 0

    // create a socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    ERROR_HELPER(socket_desc, "Could not create socket");

    // set up parameters for the connection
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(SERVER_PORT); // don't forget about network byte order!

   
    char buf[1024];
    size_t buf_len;
    
    char msg_recv[1024];
    size_t msg_recv_len=sizeof(msg_recv);
    
/*
    // display welcome message from server
    while ( (msg_len = recv(socket_desc, buf, buf_len - 1, 0)) < 0 ) {
        if (errno == EINTR) continue;
        ERROR_HELPER(-1, "Cannot read from socket");
    }
    buf[msg_len] = '\0';
    printf("%s", buf);
  */  

	 // initiate a connection on the socket
	ret = connect(socket_desc, (struct sockaddr*) &server_addr, sizeof(struct sockaddr_in));
	ERROR_HELPER(ret, "Could not create connection");
	if (DEBUG) fprintf(stderr, "Connection established!\n");
	printf("/create or /join <name_channel>\n");
	while(1){
		printf("send: ");
		gets(buf);
		buf_len = strlen(buf);
		
		
		 while ( (ret = send(socket_desc, buf, buf_len, 0)) < 0 ) {
				if (errno == EINTR) continue;
				ERROR_HELPER(-1, "Cannot write to the socket");
		}
	
		while ( (ret = recv(socket_desc, msg_recv, msg_recv_len, 0)) < 0 ) {
            if (errno == EINTR) continue;
            ERROR_HELPER(-1, "Cannot read from socket");
        }
        printf("recv: %s\n",msg_recv);
		

	}

    // close the socket
    ret = close(socket_desc);
    ERROR_HELPER(ret, "Cannot close socket");

    if (DEBUG) fprintf(stderr, "Exiting...\n");

    exit(EXIT_SUCCESS);
    
}
