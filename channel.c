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

void* connection_handler(void* arg) {
    handler_args_t* args = (handler_args_t*)arg;
	channel_list_struct* channel_list = (args->channel_list); //mi salvo la channel_list_struct (per alleggerire la scrittura) - troppe concatenazioni con ->
	channel_struct* my_channel;
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
	
	/*			thead_arg			channel_list_strunct     array			channel_struct		array 
	 * 
	* args ->	├─channel_list* ->  ├─channel -> 	   		├─channel -> 	├─dim
	*	  	 	├─socket_desc		├─num_channels			├─channel		├─client ->		client_desc,client_desc...
	* 			.....				├ ....					├ ....			├─....
	* 								....					....			....
	*/

	
    // parse client IP address and port
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(args->client_addr->sin_addr), client_ip, INET_ADDRSTRLEN);
    uint16_t client_port = ntohs(args->client_addr->sin_port); // port number is an unsigned short
	
	// read message from client
	while ( (recv_bytes = recv(args->socket_desc, buf, buf_len, 0)) < 0 ) {
		if (errno == EINTR) continue;
		ERROR_HELPER(-1, "Cannot read from socket");
	}
	
	/** TODO: distinguere dai comandi i messaggi scritti dal client ed inoltrarli a tutti gli altri client connessi nel canale*/
	
	/**TODO: fare il log**/
	
	// check if create 
	if (recv_bytes == create_command_len && !memcmp(buf, create_command, create_command_len)){
		if(DEBUG) printf("create new channel\n");
		
		ret=sem_wait(sem);
		ERROR_HELPER(ret,"error sem_wait");
		
		//creo un nuovo canale e setto la struttura dati che lo rappresenta
		channel_list -> num_channels = channel_list -> num_channels+1; //aumento di 1 il numero di canali
		channel_list -> channel = (channel_struct*) realloc (channel_list->channel, sizeof(channel_struct)*(channel_list->num_channels));  
		
		my_channel = &channel_list->channel[channel_list -> num_channels-1];  //prendo il mio canale (cioè l'ultimo della lista) e lo copio per comodità
		
		my_channel -> dim = 1;  								//quando si crea il canale c'è solo il proprietario
		my_channel -> client_desc = (int*)malloc(sizeof(int)); 	//allocazione dinamica
		my_channel -> client_desc[0] = args -> socket_desc; 	//aggiungo il proprietario ai client connessi al canale
		my_channel -> owner = args -> socket_desc; 				//setto il proprietario
		my_channel -> id = 0;									//setto un id al canale. Per ora a tutti zero 
		/**TODO: decidere come gestire gli id dei canali**/
		/* IDEA!
		 *  id=posizione nell'array channel_list
		 * 
		 */
		 		
		//per ora faccio solo una stampa
		if(DEBUG) printChannel(my_channel); //stampa di debug del canale
		
		ret=sem_post(sem);
		ERROR_HELPER(ret,"error sem_post");
				
			
	}
	// check if join
	if (recv_bytes == join_command_len && !memcmp(buf, join_command, join_command_len)){
		printf("join canale\n");		
		/**TODO: join al canale**/
		
	}
	
	/**TODO: gestire altri comandi (e.g. QUIT, DELETE, ecc...)**/
	
	
    // close socket
    ret = close(args->socket_desc);
    ERROR_HELPER(ret, "Cannot close socket for incoming connection");

    if (DEBUG) fprintf(stderr, "Thread created to handle the request has completed.\n");

    free(args->client_addr); // do not forget to free this buffer!
    free(args);

    pthread_exit(NULL);
}

void printChannel(channel_struct* channel){
	printf("\nCHANNEL\n");
	printf("id: %d\n",channel->id);
	printf("owner: %d\n",channel->owner);
	printf("dimension: %d\n",channel->dim);
	printf("client_desc: ");
	for(int i=0;i<channel->dim;i++)printf("%d, ",channel->client_desc[i]);
	printf("\n");
}
