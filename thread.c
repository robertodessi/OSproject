#include "thread.h"
#include "common.h"

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
	  
    handler_args_t* args = (handler_args_t*) arg;
  
    channel_struct* my_channel=NULL;  //channel_list_struct* channel_list;
    
    sem_t* my_named_semaphore=NULL;  //semaforo
	char* name_channel=NULL; 		 //nome del canale
    
    int ret, recv_bytes;
    
    int connect = 0; //flag che indica se il client è connesso ad un canale oppure no
    int command = 0; //flag che indica se il client ha inviato un comando oppure no
	
	//buffer per i recv
    char buf[1024];
    size_t buf_len = sizeof (buf);
	    

    /**COMMAND**/

    //CREATE
    char* create_command = CREATE_COMMAND;
    size_t create_command_len = strlen(create_command);

    //JOIN
    char* join_command = JOIN_COMMAND;
    size_t join_command_len = strlen(join_command);

    //QUIT
    char* quit_command = QUIT_COMMAND;
    size_t quit_command_len = strlen(quit_command);

    /**END_COMMAND **/


    // parse client IP address and port
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(args->client_addr->sin_addr), client_ip, INET_ADDRSTRLEN);
    //uint16_t client_port = ntohs(args->client_addr->sin_port); // port number is an unsigned short

    while (1) {
				
        // read message from client
        recv_bytes = ricevi(buf,buf_len,args->socket_desc);
        
        command=0;  //setto il flag a false
	
		
        /**TODO: fare il log**/

        // ****************************   CREATE   ********************************************
        /**  /create <name_channel>  **/
        if (!connect && !memcmp(buf, create_command, create_command_len)) {            
            if (DEBUG) printf("try to create new channel\n");

            char* name_channel = prendiNome(buf, recv_bytes + 1,create_command_len); //prendo il nome del canale			
			            
            command=1; //setto il flag a true                                    

            name_channel = prendiNome(buf, recv_bytes + 1,create_command_len); //prendo il nome del canale	
            

			//controllo che il nome non sia vuoto
			if(strlen(name_channel)==0){
				invio("il nome del canale non può essere vuoto\0",args->socket_desc);				 
				continue;
			}	

			/**INIZIO SEZIONE CRITICA**/
			ret = sem_wait(sem);
			ERROR_HELPER(ret, "error sem_wait");
			
			//channel_list = *(args->channel_list); //mi salvo la channel_list_struct (per comodità) 

			int i=0;
			int nameIsPresent=0;  //booleano che indica se un nome è già stato preso oppure no
			
			//controllo che il nome non sia già stato usato per un altro canale			
			while(i < args->channel_list->num_channels){
				if(strcmp(name_channel,args->channel_list->name_channel[i])==0){ //equals
					nameIsPresent=1;  //se è presente setto il booleano a vero
					break;					
				}
				i++;
			}
			
			//se è il nome è già stato preso avviso il client
			if(nameIsPresent){
				invio("il nome del canale esiste già, scegline un altro\0",args->socket_desc);
				ret = sem_post(sem);
				ERROR_HELPER(ret, "error sem_post");
				//FINE SEZIONE CRITICA//				
				continue;  //non creo il cananale, quindi skippo il resto
			}

						
			if (DEBUG) printf("create new channel\n");

			my_channel = (channel_struct*) malloc(sizeof (channel_struct));
			my_channel -> dim = 1; //quando si crea il canale c'è solo il proprietario
			my_channel -> client_desc = (int*) malloc(sizeof (int)); //allocazione dinamica
			my_channel -> client_desc[0] = args->socket_desc; //aggiungo il proprietario ai client connessi al canale
			my_channel -> owner = args->socket_desc; //setto il proprietario
			my_channel -> name_channel = name_channel;  
			/*WARNING!! 
			 * il nome del canale è ridondante!
		     */
			my_channel -> id = 0; //setto un id al canale. Per ora a tutti zero 		


			// aggiungo my_channel alla lista dei canali (channel_list)
			int n=++(args->channel_list->num_channels);  //uso n per rendere il codice più leggibile
		
			args->channel_list->name_channel = (char**) realloc(args->channel_list->name_channel,n*sizeof(char*));  
			args->channel_list->name_channel[n-1]=name_channel;	//aggiungo il nuovo nome
			
			args->channel_list->channel = (channel_struct**) realloc(args->channel_list->channel,n*sizeof(channel_struct*));  
			args->channel_list->channel[n-1] = my_channel;	//aggiungo il nuovo canale		
			
			//args->channel_list->sem_channel = (sem_t*) realloc(args->channel_list->sem_channel,n*sizeof(sem_t));  
			my_named_semaphore = sem_open(name_channel, O_CREAT|O_EXCL, 0666, 1);   //aggiungo il nuovo semaforo
			if (my_named_semaphore == SEM_FAILED && errno == EEXIST) {
				printf("already exists, let's unlink it...\n");
				sem_unlink(name_channel);
				printf("and then reopen it...\n");
				my_named_semaphore = sem_open(name_channel, O_CREAT|O_EXCL, 0666, 1);
			}
			if (my_named_semaphore == SEM_FAILED) {
				ERROR_HELPER(-1, "[FATAL ERROR] Could not create a semaphore");
			}												

			ret = sem_post(sem);
			ERROR_HELPER(ret, "error sem_post");
			/**FINE SEZIONE CRITICA**/

			if (DEBUG) printChannel(my_channel);
			invio("canale creato con successo\0",args->socket_desc);

			connect = 1;	//setto il flag di connessione a true	

            }
            
        // ****************************   JOIN   ********************************************
        /**  /join <name_channel>  **/
        if (!connect && !memcmp(buf, join_command, join_command_len)) {
            if (DEBUG) printf("try to join channel\n");  
            
            command=1; //setto il flag a true                      

            name_channel = prendiNome(buf, recv_bytes + 1,join_command_len); //prendo il nome del canale	
         	
			//controllo che il nome non sia vuoto
			if(strlen(name_channel)==0){
				invio("il nome del canale non può essere vuoto\0",args->socket_desc);						 
				continue;
			}	

			/**INIZIO SEZIONE CRITICA PER LA LISTA**/
			ret = sem_wait(sem);
			ERROR_HELPER(ret, "error sem_wait");
		

			int i=0;
			int nameIsPresent=0;  //booleano che indica se un nome è già stato preso oppure no
			//controllo che il nome non sia già stato usato per un altro canale
			
			
			while(i < args->channel_list->num_channels){
				if(strcmp(name_channel,args->channel_list->name_channel[i])==0){ //equals
					nameIsPresent=1;  //se è presente setto il booleano a vero
					break;					
				}
				i++;
			}
						
			//se il canale esiste
			if(nameIsPresent){
				my_named_semaphore = sem_open(name_channel, 0); // mode is 0: sem_open is not allowed to create it!
				/**INIZIO SEZIONE CRITICA PER IL CANALE**/
				ret = sem_wait(my_named_semaphore);
				ERROR_HELPER(ret, "error sem_wait");
				
				my_channel = args->channel_list->channel[i];  //salvo in my_channel il canale in cui sono attualmente collegato
				
				//aggiorno la struttura dati del canale	
				int dim = ++(my_channel->dim);  //aumento di 1 il numero di parteciapnti al canale
				my_channel->client_desc=(int*)realloc(my_channel->client_desc, dim*sizeof(int));
				my_channel->client_desc[dim-1] = args->socket_desc; 	//aggiungo il nuovo membro al canale			
				
				
				ret = sem_post(my_named_semaphore);
				ERROR_HELPER(ret, "error sem_post");
				/**FINE SEZIONE CRITICA PER IL CANALE**/
				
				invio("connesso al canale\0",args->socket_desc);		
				
				printChannel(args->channel_list->channel[i]);
				connect = 1;				
				
			}
			else{	//se il canale non esiste
				invio("il canale non esiste\0",args->socket_desc);
			}
			
			ret = sem_post(sem);
			ERROR_HELPER(ret, "error sem_post");
			/**FINE SEZIONE CRITICA PER IL CANALE**/
            
        }
        
        // ****************************   QUIT   ********************************************
        if (connect && recv_bytes == quit_command_len && !memcmp(buf, quit_command, quit_command_len)) {
            if (DEBUG) printf("quit canale\n");
            
            command=1;
            
            invio("quit dal canale\0",args->socket_desc);
            
            //chi esce NON è il propretario del canale
            if (my_channel->owner != args->socket_desc) {

                /**TODO: cancellare da my_channel il suo descrittore*/

            } else {
                /**TODO: gestire in caso in cui il creatore esce dal canale**/
            }

            connect = 0;
        }
        /**TODO: gestire altri comandi (e.g. /quit, /delete, ecc...)**/
       
        //inoltro dei messaggi
        if(connect && !command){
			if(DEBUG) printf("inoltro\n");
			int i=0;
			for(i=0; i < my_channel->dim; i++){  //inoltro del messaggio escuso se sesso
				if(my_channel->client_desc[i] != args->socket_desc) invio(buf,my_channel->client_desc[i]);
			}
			
		}
    }

    // close socket
    ret = close(args->socket_desc);
    ERROR_HELPER(ret, "Cannot close socket for incoming connection");

    if (DEBUG) fprintf(stderr, "Thread created to handle the request has completed.\n");

    free(args->client_addr); // do not forget to free this buffer!
    free(args);

    pthread_exit(NULL);
}

/**TODO: togliere gli spazi alla fine del nome **/
char* prendiNome(char* str, int len, size_t command_len) {
    char* res = (char*) malloc(sizeof(char) * (len-command_len));
    /* Warning: 
     * in questo momento il comando funziona anche senza spazio tra il comando e il nome (es:  /create<nome_canale>) 
     * funziona anche se ci sono più spazi */
    int index=command_len;
    int i = 0;
    while (str[index] == ' ') index++;  //tolgo gli eventuali spazi tra il comando e il nome del canale  
    for (; i <len ; i++) res[i] = str[index++];
    res[++i] = '\0';
    return res;
}

void printChannel(channel_struct* channel) {
    printf("\nCHANNEL\n");
    printf("name: %s\n", channel->name_channel);
    printf("id: %d\n", channel->id);
    printf("owner: %d\n", channel->owner);
    printf("dimension: %d\n", channel->dim);
    printf("client_desc: ");
    for (int i = 0; i < channel->dim; i++)printf("%d, ", channel->client_desc[i]);
    printf("\n\n");
}

void invio(char* s, int dest){	
	int ret;
   	while ( (ret = send(dest, s, sizeof(char)*strlen(s)+1, 0)) < 0 ) {
		if (errno == EINTR) continue;
		ERROR_HELPER(-1, "Cannot write to the socket");
	}
}

int ricevi(char* buf, size_t buf_len, int mitt){
	int recv_bytes=0;
	while ((recv_bytes = recv(mitt, buf, buf_len, 0)) < 0) {
		if (errno == EINTR) continue;
        ERROR_HELPER(-1, "Cannot read from socket");
    }
    return recv_bytes;
}
