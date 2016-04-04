#include "common.h"
#include "log.h"
#include "channel.h"

#include <stdio.h>//printf
#include <stdlib.h>//warning exit
#include <errno.h>//per poter leggere il valore di errno
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>  // htons()
#include <unistd.h> 
#include <pthread.h>
#include <semaphore.h>





//====================================
//			MAIN
//====================================



int main(int argc, char *argv[]) {
	
    //	error check descriptor 
    int ret;

    //	socekt descriptor
    int socket_desc, client_desc;
    
    // some fields are required to be filled with 0
    struct sockaddr_in server_addr = {0};

    int sockaddr_len = sizeof(struct sockaddr_in); // we will reuse it for accept()
    
    //array che rappresenta la lista di tutti i canali
	channel_struct* channel_list=(channel_struct*)malloc(0);
	
	//numero di canali presenti
	int num_channel=0; 
	
	//array di semafori, uno per ogni canale	
	sem_t* sem=(sem_t*)malloc(0);
	
    
    /**COMMAND**/
    
	//CREATE
    char* create_command = CREATE_COMMAND;
    size_t create_command_len = strlen(create_command);
    
    //JOIN
    char* join_command = JOIN_COMMAND;
    size_t join_command_len = strlen(join_command);
    
	/**END_COMMAND **/
	
	
	int recv_bytes;

    char buf[1024];
    size_t buf_len = sizeof(buf);
    
    
      

    /*
    ret = sigfillset(&maschera);
    ERROR_HELPER(ret, "Errore nella sigfillset\n\n");

    ret = sigdelset(&maschera, SIGTERM);
    ERROR_HELPER(ret, "Errore nella sigdelset(SIGTERM)\n\n");

    ret = sigdelset(&maschera, SIGINT);
    ERROR_HELPER(ret, "Errore nella sigdelset(SIGINT)\n\n");

    ret = sigdelset(&maschera, SIGQUIT);
    ERROR_HELPER(ret, "Errore nella sigdelset(SIGQUIT)\n\n");

    ret = sigdelset(&maschera, SIGHUP);
    ERROR_HELPER(ret, "Errore nella sigdelset(SIGHUP)\n\n");

    ret = sigdelset(&maschera, SIGPIPE);
    ERROR_HELPER(ret, "Errore nella sigdelset(SIGPIPE)\n\n");

    ret = sigdelset(&maschera, SIGALRM);
    ERROR_HELPER(ret, "Errore nella sigdelset(SIGALRM)\n\n");

    ret = sigprocmask(SIG_BLOCK, &maschera, NULL);
    ERROR_HELPER(ret, "Errore nella sigprocmask\n\n");

    gestione_segnali(SIGTERM, gestione_chiusura, 0);
    gestione_segnali(SIGINT, gestione_chiusura, 0);
    gestione_segnali(SIGQUIT, gestione_chiusura, 0);
    gestione_segnali(SIGHUP, gestione_chiusura, 0);
    gestione_segnali(SIGPIPE, gestione_chiusura_inattiva, 0);
    gestione_segnali(SIGALRM, gestione_timer, 0);
     */
    
    //Presentation
    printf("Server: Welcome by ChatApp!\n");
    printf("CRT+C to kill your Server\n");
    
    resetLog(); //resetto il file di log
    logMsg("Server started to run");

    // initialize socket for listening
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    //socket_desc = -1;
    ERROR_HELPER(socket_desc, "Errore creazione socket\n\n");
	
    server_addr.sin_addr.s_addr = INADDR_ANY; // we want to accept connections from any interface
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(SERVER_PORT); // don't forget about network byte order!
    
    /* We enable SO_REUSEADDR to quickly restart our server after a crash:
     * for more details, read about the TIME_WAIT state in the TCP protocol */
    int reuseaddr_opt = 1;
    ret = setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_opt, sizeof(reuseaddr_opt));
    ERROR_HELPER(ret, "Cannot set SO_REUSEADDR option");

    // bind address to socket
    ret = bind(socket_desc, (struct sockaddr*) &server_addr, sockaddr_len);
    ERROR_HELPER(ret, "Cannot bind address to socket");

    // start listening
    ret = listen(socket_desc, MAX_CONN_QUEUE);
    ERROR_HELPER(ret, "Cannot listen on socket");
    
    // we allocate client_addr dynamically and initialize it to zero
    struct sockaddr_in* client_addr = calloc(1, sizeof(struct sockaddr_in));
    
    
    // loop to manage incoming connections forking the server process
    while (1) {
        // accept incoming connection
        client_desc = accept(socket_desc, (struct sockaddr*) client_addr, (socklen_t*) &sockaddr_len);
        if (client_desc == -1 && errno == EINTR) continue; // check for interruption by signals
        ERROR_HELPER(client_desc, "Cannot open socket for incoming connection");

        if (DEBUG) fprintf(stderr, "Incoming connection accepted...\n");
        logMsg("utente connesso");
        
        // read message from client
		while ( (recv_bytes = recv(client_desc, buf, buf_len, 0)) < 0 ) {
			if (errno == EINTR) continue;
			ERROR_HELPER(-1, "Cannot read from socket");
		}
		
		/**TODO: gestire il log**/
		// check if create 
		if (recv_bytes == create_command_len && !memcmp(buf, create_command, create_command_len)){
			if(DEBUG) printf("create new channel\n");
			
			//creo un nuovo canale e setto la struttura dati che lo rappresenta
			channel_list=(channel_struct*)realloc(channel_list,sizeof(channel_struct)*(++num_channel));
			channel_list[num_channel-1] . dim=1;  //quando si crea il canale c'è solo il proprietario
			channel_list[num_channel-1] . client_desc=(int*)malloc(sizeof(int)); //allocazione dinamica
			channel_list[num_channel-1] . client_desc[0]=client_desc; //aggiungo il proprietario ai client connessi al canale
			channel_list[num_channel-1] . owner=client_desc; //setto il proprietario
			channel_list[num_channel-1] . id=0; //setto un id al canale. Per ora a tutti zero 
			/**TODO: decidere come gestire gli id dei canali**/
			/* IDEA!
			 *  id=posizione nell'array channel_list
			 * 
			 */
			 
			//creo il semaforo per il canale
			sem=(sem_t*)realloc(sem,sizeof(sem_t)*(num_channel));
			
			/**TODO: inizializzare il semaforo e aprirlo**/
			
			//creo il thread che gestirà il client da ora in avanti
			pthread_t thread;

			// put arguments for the new thread into a buffer
			handler_args_t* thread_args = malloc(sizeof(handler_args_t));
			thread_args -> socket_desc = client_desc;
			thread_args -> client_addr = client_addr;
			thread_args -> sem = &sem[num_channel-1];
			thread_args -> channel = &channel_list[num_channel-1];
		   

			if (pthread_create(&thread, NULL, welcome_handler, (void*)thread_args) != 0) {
				fprintf(stderr, "Can't create a new thread, error %d\n", errno);
				exit(EXIT_FAILURE);
			}

			if (DEBUG) fprintf(stderr, "New thread created to handle the request!\n");

			pthread_detach(thread); // I won't phtread_join() on this thread			
			
		}
		// check if join
		if (recv_bytes == join_command_len && !memcmp(buf, join_command, join_command_len)){
			printf("join canale\n");
			
			/**TODO: join al canale**/
			
		}
		
		/**TODO: gestire altri comandi (e.g. SHOW ALL CHANNEL)**/
		
        // we can't just reset fields: we need a new buffer for client_addr!
		client_addr = calloc(1, sizeof(struct sockaddr_in));
        
    }

    exit(EXIT_SUCCESS); // this will never be executed

}

//lo appunto qui perchè così me lo ricordo  http://www.dis.uniroma1.it/~aniello/sc/m2/es10/sol/chat_socket.c

/* select() uses sets of descriptors and a timeval interval. The
     * methods returns when either an event occurs on a descriptor in
     * the sets during the given interval, or when that time elapses.
     *
     * The first argument for select is the maximum descriptor among
     * those in the sets plus one. Note also that both the sets and
     * the timeval argument are modified by the call, so you should
     * reinitialize them across multiple invocations.
     *
     * On success, select() returns the number of descriptors in the
     * given sets for which data may be available, or 0 if the timeout
     * expires before any event occurs. 
     */
     
    
