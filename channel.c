#include "common.h"
#include "channel.h"

#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>  // htons()
#include <pthread.h>
#include <unistd.h> //close()

//====================================
//			CHANNEL
//====================================

void* welcome_handler(void* arg) {
    handler_args_t* args = (handler_args_t*)arg;
	
    
    int ret, recv_bytes;

    char buf[1024];
    size_t buf_len = sizeof(buf);
    size_t msg_len;
	
	/**COMMAND**/
	//CREATE
    char* create_command = CREATE_COMMAND;
    size_t create_command_len = strlen(create_command);
    //JOIN
    char* join_command = JOIN_COMMAND;
    size_t join_command_len = strlen(join_command);
	/**END_COMMAND **/
	
    // parse client IP address and port
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(args->client_addr->sin_addr), client_ip, INET_ADDRSTRLEN);
    uint16_t client_port = ntohs(args->client_addr->sin_port); // port number is an unsigned short

    
    // read message from client
	while ( (recv_bytes = recv(args->socket_desc, buf, buf_len, 0)) < 0 ) {
		if (errno == EINTR) continue;
		ERROR_HELPER(-1, "Cannot read from socket");
	}
	channel_struct* channel={0};
	// check if create 
    if (recv_bytes == create_command_len && !memcmp(buf, create_command, create_command_len)){
		if(DEBUG) printf("create new channel\n");
		
		//creo un nuovo canale e setto la struttura dati che lo rappresenta
		channel=(channel_struct*)malloc(sizeof(channel_struct));
		channel->dim=0;
		channel->dim=(channel->dim)+1;
		channel->client_desc=(int*)realloc(channel->client_desc,channel->dim*sizeof(int));
		channel->client_desc[(channel->dim)-1]=args->socket_desc;
		channel->owner=args->socket_desc;
		channel->id=pthread_self();
		
		if(DEBUG) printChannel(channel); //stampa di debug del canale
		free(channel->client_desc); // do not forget to free this buffer!
		free(channel);
	}
	// check if join
    if (recv_bytes == join_command_len && !memcmp(buf, join_command, join_command_len)){
		printf("join canale\n");
		
		/**TODO: join al canale**/
		
	}
	
    // close socket
    ret = close(args->socket_desc);
    ERROR_HELPER(ret, "Cannot close socket for incoming connection");

    if (DEBUG) fprintf(stderr, "Thread created to handle the request has completed.\n");

    free(args->client_addr); // do not forget to free this buffer!
    free(args);
   
    pthread_exit(NULL);
}

void printChannel(channel_struct* channel){
	printf("CHANNEL\n");
	printf("id: %d\n",channel->id);
	printf("owner: %d\n",channel->owner);
	printf("dimension: %d\n",channel->dim);
	printf("client_desc: ");
	for(int i=0;i<channel->dim;i++)printf("%d, ",channel->client_desc[i]);
	printf("\n");
}
