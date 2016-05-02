#include <errno.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>  // htons() and inet_addr()
#include <netinet/in.h> // struct sockaddr_in
#include <sys/socket.h>
#include <pthread.h>

#include "common.h"

//struttira dati per i thread
typedef struct{
	int desc;  //descrittore della socket	
}struct_arg_client;


//funzione che esegue il thread_send
void* invia(void* arg){
	struct_arg_client* args = (struct_arg_client*) arg;
	
	int ret;	
	char buf[1024];
       
	while(1){
		printf("send: ");
		fgets(buf, 1024, stdin);  //fgets prende anche il carattere invio
		size_t buf_len = strlen(buf);
		--buf_len; // remove '\n' from the end of the message

		while ( (ret = send(args->desc, buf,buf_len, 0)) < 0 ) {
			if (errno == EINTR) continue;
			ERROR_HELPER(-1, "Cannot write to the socket");
		}
	}
	
} 

//funzione che esegue il thread_rcv
void* ricevi(void* arg){
	struct_arg_client* args = (struct_arg_client*) arg;
	
	char msg_recv[1024];
    size_t msg_recv_len=sizeof(msg_recv);
	while(1){
		int recv_bytes=0;
		while ((recv_bytes = recv(args->desc, msg_recv, msg_recv_len, 0)) < 0) {
			if (errno == EINTR) continue;
			ERROR_HELPER(-1, "Cannot read from socket");
		}
		printf("ricevuto: %s\n",msg_recv);
	}
    return NULL;
	
}

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
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT); // don't forget about network byte order!

    // initiate a connection on the socket
    ret = connect(socket_desc, (struct sockaddr*) &server_addr, sizeof (struct sockaddr_in));
    ERROR_HELPER(ret, "Could not create connection");
    if (DEBUG) fprintf(stderr, "Connection established!\n");
    printf("/create or /join <name_channel>\n");
    
    /*
    // display welcome message from server
    while ( (msg_len = recv(socket_desc, buf, buf_len - 1, 0)) < 0 ) {
        if (errno == EINTR) continue;
        ERROR_HELPER(-1, "Cannot read from socket");
    }
    buf[msg_len] = '\0';
    printf("%s", buf);
       
	//TODO check  If end-of-file is encountered and no characters have been read into the array
	
	The fgets function reads at most one less than the number of characters
	specified by n from the stream pointed to by stream into the array pointed
	to by s. No additional characters are read after a new-line character
	(which is retained) or after end-of-file. A null character is written
	immediately after the last character read into the array.
	 
	The fgets function returns s if successful. If end-of-file is encountered
	and no characters have been read into the array, the contents of the array
	remain unchanged and a null pointer is returned. If a read error occurs
	during the operation, the array contents are indeterminate and a null
	pointer is returned.
         */
        
        
    //thread dedicato per la rcv
	pthread_t thread_rcv;

	// put arguments for the new thread into a buffer
	struct_arg_client* thread_rcv_args = (struct_arg_client*) malloc(sizeof(struct_arg_client));
	thread_rcv_args -> desc =socket_desc;
	
	if (pthread_create(&thread_rcv, NULL, ricevi, (void*)thread_rcv_args) != 0) {
			fprintf(stderr, "Can't create a new thread, error %d\n", errno);
			exit(EXIT_FAILURE);
	}
        
	//thread dedicato per la send
	pthread_t thread_send;

	// put arguments for the new thread into a buffer
	struct_arg_client* thread_send_args = (struct_arg_client*) malloc(sizeof(struct_arg_client));
	thread_send_args -> desc =socket_desc;
	
	if (pthread_create(&thread_send, NULL, invia, (void*)thread_send_args) != 0) {
			fprintf(stderr, "Can't create a new thread, error %d\n", errno);
			exit(EXIT_FAILURE);
	}
	
  
	
	/**TODO: verificare il secondo paramentro: va bene lasciare NULL ?*/
	pthread_join(thread_send,NULL); 
	/**TODO: verificare il secondo paramentro: va bene lasciare NULL ?*/
	pthread_join(thread_rcv,NULL); 	

    // close the socket
    ret = close(socket_desc);
    ERROR_HELPER(ret, "Cannot close socket");

    if (DEBUG) fprintf(stderr, "Exiting...\n");

    exit(EXIT_SUCCESS);

}
