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
	int ret;
   /* 
    int ret, recv_bytes;

    char buf[1024];
    size_t buf_len = sizeof(buf);
    size_t msg_len;
	*/
	
	
    // parse client IP address and port
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(args->client_addr->sin_addr), client_ip, INET_ADDRSTRLEN);
    uint16_t client_port = ntohs(args->client_addr->sin_port); // port number is an unsigned short

    
    //per ora faccio solo una stampa
	if(DEBUG) printChannel(args -> channel); //stampa di debug del canale
	
	
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
