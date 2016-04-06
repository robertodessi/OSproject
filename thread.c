#include "thread.h"
#include "common.h"
#include "channel.h"

#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>  // htons()
#include <pthread.h>
#include <unistd.h> //close()
#include <sys/types.h>


//====================================
//			THREAD
//====================================


void* connection_handler(void* arg) {
    handler_args_t* args = (handler_args_t*)arg;
	channel_list_struct* channel_list = (args->channel_list); //mi salvo la channel_list_struct (per comodità) 
	channel_struct* my_channel;
	int ret, recv_bytes;
	int fd[2];
	char* name_channel;  //nome del canale
	int connect=0;  //flag che indica se il client è connesso ad un canale oppure no
	
    char buf[1024];
    size_t buf_len = sizeof(buf);
    
    char msg[1024];
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
    
    while(1){
		
		// read message from client
		while ( (recv_bytes = recv(args->socket_desc, buf, buf_len, 0)) < 0 ) {
			if (errno == EINTR) continue;
			ERROR_HELPER(-1, "Cannot read from socket");
		}
		
		/** TODO: distinguere dai comandi i messaggi scritti dal client ed inoltrarli a tutti gli altri client connessi nel canale*/
		
		/**TODO: fare il log**/
		
		// check if create 
		/**  /create <name_channel>  **/
		if (!connect && !memcmp(buf, create_command, create_command_len)){
			if(DEBUG) printf("try to create new channel\n");
			
			name_channel=prendiNome(buf); //prendo il nome del canale
			
			
			//accedo alla lista condivisa
			ret=sem_wait(sem);
			ERROR_HELPER(ret,"error sem_wait");
			
			/**TODO: verificare che il nome del canale sia disponibile**/
			
			ret=sem_post(sem);
			ERROR_HELPER(ret,"error sem_post"); //fine sezione critica
			
			
			if(DEBUG) printf("create new channel\n");
			
			channel_struct* my_channel=(channel_struct*) malloc(sizeof(channel_struct));
			
			ret=pipe(fd);
			ERROR_HELPER(ret,"errore apertura pipe");
			
			
			
			//creo il thread che gestirà il canale
			pthread_t thread;

			// put arguments for the new thread into a buffer
			handler_args_c* thread_args = malloc(sizeof(handler_args_c));
			thread_args -> channel_list = channel_list;  //passo il puntatore alla lista dei canali
			thread_args -> fd = fd[0];  //passe il descrittore di lettura della pipe
			thread_args -> my_channel = my_channel;  //passo il canale di cui si deve occupare
		   

			if (pthread_create(&thread, NULL, channel_handler, (void*)thread_args) != 0) {
				fprintf(stderr, "Can't create a new thread, error %d\n", errno);
				exit(EXIT_FAILURE);
			}

			if (DEBUG) fprintf(stderr, "New thread created to handle the request!\n");

			pthread_detach(thread); // I won't phtread_join() on this thread	
			
			//accedo alla lista condivisa
			ret=sem_wait(sem);
			ERROR_HELPER(ret,"error sem_wait");
			
			
			/**	TODO: **/
			
			ret=sem_post(sem);
			ERROR_HELPER(ret,"error sem_post");
			
			connect=1;
				
		}
		// check if join
		/**  /join <name_channel>  **/
		if (!connect && recv_bytes == join_command_len && !memcmp(buf, join_command, join_command_len)){
			printf("join canale\n");		
			/**TODO: join al canale**/
			connect=1;
		}
		
		/**TODO: gestire altri comandi (e.g. QUIT, DELETE, ecc...)**/
	}
	
    // close socket
    ret = close(args->socket_desc);
    ERROR_HELPER(ret, "Cannot close socket for incoming connection");

    if (DEBUG) fprintf(stderr, "Thread created to handle the request has completed.\n");

    free(args->client_addr); // do not forget to free this buffer!
    free(args);

    pthread_exit(NULL);
}

char* prendiNome(char* str){
	char res[MAX_LENGHT]="prova";
	/**TODO: scrivere la funzione che prende il nome**/
	return res;
}
