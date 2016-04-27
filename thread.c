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
    channel_list_struct* channel_list = (args->channel_list); //mi salvo la channel_list_struct (per comodità) 
    channel_struct* my_channel;

    int ret, recv_bytes;
    char* name_channel=NULL; //nome del canale

    int connect = 0; //flag che indica se il client è connesso ad un canale oppure no
	
	//buffer per i recv
    char buf[1024];
    size_t buf_len = sizeof (buf);
	
	//buffer per i send
    char msg[1024];
    size_t msg_len;


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
    uint16_t client_port = ntohs(args->client_addr->sin_port); // port number is an unsigned short

    while (1) {

        // read message from client
        while ((recv_bytes = recv(args->socket_desc, buf, buf_len, 0)) < 0) {
            if (errno == EINTR) continue;
            ERROR_HELPER(-1, "Cannot read from socket");
        }

        /** TODO: distinguere dai comandi i messaggi scritti dal client ed inoltrarli a tutti gli altri client connessi nel canale*/

        /**TODO: fare il log**/

        // check if create 
        /**  /create <name_channel>  **/
        if (!connect && !memcmp(buf, create_command, create_command_len)) {
            if (DEBUG) printf("try to create new channel\n");

            name_channel = prendiNome(buf, recv_bytes + 1,create_command_len); //prendo il nome del canale
			
			//controllo che il nome non sia vuoto
			if(strlen(name_channel)==0){				 
				 strcpy(msg,"il nome del canale non può essere vuoto\0");
				 while ( (ret = send(args->socket_desc, msg, sizeof(char)*strlen(msg)+1, 0)) < 0 ) {
						if (errno == EINTR) continue;
						ERROR_HELPER(-1, "Cannot write to the socket");
				}
				continue;
			}
		
	

			/**INIZIO SEZIONE CRITICA**/
			ret = sem_wait(sem);
			ERROR_HELPER(ret, "error sem_wait");

			int i=0;
			int nameIsPresent=0;  //booleano che indica se un nome è già stato preso oppure no
			//controllo che il nome non sia già stato usato per un altro canale
			while(i < channel_list->num_channels){
				if(strcmp(name_channel,channel_list->name_channel[i])==0){ //equals
						nameIsPresent=1;  //se è presente setto il booleano a vero
						break;					
				}
				i++;
			}
			//se è il nome è già stato preso avviso il client
			if(nameIsPresent){
				strcpy(msg,"il nome del canale esiste già, scegline un altro\0");
				 while ( (ret = send(args->socket_desc, msg, sizeof(char)*strlen(msg)+1, 0)) < 0 ) {
						if (errno == EINTR) continue;
						ERROR_HELPER(-1, "Cannot write to the socket");
				} 
				nameIsPresent=0;
				continue;
			}

			ret = sem_post(sem);
			ERROR_HELPER(ret, "error sem_post"); 
			/**FINE SEZIONE CRITICA**/


			if (DEBUG) printf("create new channel\n");

				channel_struct* my_channel = (channel_struct*) malloc(sizeof (channel_struct));
				my_channel -> dim = 1; //quando si crea il canale c'è solo il proprietario
				my_channel -> client_desc = (int*) malloc(sizeof (int)); //allocazione dinamica
				my_channel -> client_desc[0] = args->socket_desc; //aggiungo il proprietario ai client connessi al canale
				my_channel -> owner = args->socket_desc; //setto il proprietario
				my_channel -> name_channel = name_channel;
				my_channel -> id = 0; //setto un id al canale. Per ora a tutti zero 		


			
			    /**INIZIO SEZIONE CRITICA**/
				ret = sem_wait(sem);
				ERROR_HELPER(ret, "error sem_wait");


				// aggiungo my_channel alla lista dei canali (channel_list)
				int n=++(channel_list->num_channels);  //uso n per rendere il codice più leggibile
				args->channel_list->name_channel = (char**) realloc(channel_list->name_channel,n*sizeof(char*));  
				channel_list->name_channel[n]=name_channel;	//aggiungo il nuovo nome
				
				channel_list->channel = (channel_struct*) realloc(channel_list->channel,sizeof(channel_struct)*(n));  
				channel_list->channel[n] = *my_channel;	//aggiungo il nuovo canale			
				/** TODO: creare il semaforo per il canale (sem_channel)**/				
							

				ret = sem_post(sem);
				ERROR_HELPER(ret, "error sem_post");
				/**FINE SEZIONE CRITICA**/

				if (DEBUG) printChannel(my_channel);
				
				strcpy(msg,"canale creato con successo\0");
				while ( (ret = send(args->socket_desc, msg, sizeof(char)*strlen(msg)+1, 0)) < 0 ) {
						if (errno == EINTR) continue;
						ERROR_HELPER(-1, "Cannot write to the socket");
				} 

				connect = 1;	//setto il flag di connessione a true	

            }
        // check if join
        /**  /join <name_channel>  **/
        if (!connect && recv_bytes == join_command_len && !memcmp(buf, join_command, join_command_len)) {
            printf("join canale\n");
            /**TODO: join al canale**/
            connect = 1;
        }
        //check if quit
        if (connect && recv_bytes == quit_command_len && !memcmp(buf, quit_command, quit_command_len)) {
            printf("quit canale\n");

            //chi esce NON è il propretario del canale
            if (my_channel->owner != args->socket_desc) {

                /**TODO: cancellare da my_channel il suo descrittore*/

            } else {
                /**TODO: gestire in caso in cui il creatore esce dal canale**/
            }

            connect = 0;
        }
        /**TODO: gestire altri comandi (e.g. /quit, /delete, ecc...)**/
    }

    // close socket
    ret = close(args->socket_desc);
    ERROR_HELPER(ret, "Cannot close socket for incoming connection");

    if (DEBUG) fprintf(stderr, "Thread created to handle the request has completed.\n");

    free(args->client_addr); // do not forget to free this buffer!
    free(args);

    pthread_exit(NULL);
}


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
