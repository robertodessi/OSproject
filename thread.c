#include "thread.h"
#include "common.h"

#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>  // htons()
#include <pthread.h>
#include <unistd.h> //close()
#include <sys/types.h>
#include <sys/msg.h>




//====================================
//			THREAD
//====================================




void* connection_handler(void* arg) {
	  
    handler_args_t* args = (handler_args_t*) arg; 
    
    
mymsg  recv_message; //messaggio ricevuto dalla coda dei messaggi
int id_coda;  //id della coda di messaggi di questo thread
int key;  //chiave per la coda (vedi: msgget)
sem_t* my_named_semaphore;  //nome del semaforo del canale

int is_connect; //flag che indica se il client è connesso ad un canale oppure no
channel_struct* my_channel;  //channel_list_struct* channel_list;
	
    my_named_semaphore=NULL;  //semaforo
	char* name_channel=NULL; 		 //nome del canale
    
    int i,ret, recv_bytes;    
    
    int command = 0; //flag che indica se il client ha inviato un comando oppure no
    
	is_connect = 0; 
	my_channel=NULL; 
	
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
    
    //DELETE
    char* delete_command = DELETE_COMMAND;
    size_t delete_command_len = strlen(delete_command);

    /**END_COMMAND **/


    // parse client IP address and port
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(args->client_addr->sin_addr), client_ip, INET_ADDRSTRLEN);
    //uint16_t client_port = ntohs(args->client_addr->sin_port); // port number is an unsigned short
    
    key=args->socket_desc;
    
    id_coda = msgget(key, IPC_CREAT|IPC_EXCL|0666);
    if( id_coda == -1){
		id_coda = msgget(key, IPC_CREAT|0666);
		ret = msgctl(id_coda,IPC_RMID,0);
		id_coda = msgget(key, IPC_CREAT|IPC_EXCL|0666);
		if( id_coda == -1 ){
			printf("cannot install server queue, please check with the problem\n");
			exit(-1);
		}
	}
	
	printf("\tla mia %d\n",id_coda);

    while (1) {
		
		if(DEBUG) printList(args->channel_list);
			
        // read message from client
        recv_bytes = ricevi(buf,buf_len,args->socket_desc,id_coda,&recv_message, &is_connect, my_named_semaphore, my_channel, key);
        //if(DEBUG) printf("buf ricevuto=%s|\n",buf);
        
        command=0;  //setto il flag a false
	
		
        /**TODO: fare il log**/ 
        

        // ****************************   CREATE   ********************************************
        /**  /create <name_channel>  **/
        if (!is_connect && !memcmp(buf, create_command, create_command_len)) {            
            if (DEBUG) printf("try to create new channel\n");
		            
            command=1; //setto il flag a true                                    

            name_channel = prendiNome(buf, recv_bytes + 1,create_command_len); //prendo il nome del canale	
            if (DEBUG)printf("nome letto=%s|\n",name_channel);

			//controllo che il nome non sia vuoto
			if(strlen(name_channel)==0){
				invio("il nome del canale non può essere vuoto\0",args->socket_desc);				 
				continue;
			}	

			/**INIZIO SEZIONE CRITICA PER LA LISTA**/
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
		     /** TODO: l'id non serve a nulla*/
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
			/**FINE SEZIONE CRITICA PER LA LISTA**/

			if (DEBUG) printChannel(my_channel);
			invio("canale creato con successo\0",args->socket_desc);

			is_connect = 1;	//setto il flag di connessione a true	

            }
            
        // ****************************   JOIN   ********************************************
        /**  /join <name_channel>  **/
        if (!is_connect && !memcmp(buf, join_command, join_command_len)) {
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
				is_connect = 1;				
				
			}
			else{	//se il canale non esiste
				invio("il canale non esiste\0",args->socket_desc);
			}
			
			ret = sem_post(sem);
			ERROR_HELPER(ret, "error sem_post");
			/**FINE SEZIONE CRITICA PER LA LISTA**/
            
        }
        
        // ****************************   QUIT   ********************************************
        /**    /quit    */
        if (is_connect && recv_bytes == quit_command_len && !memcmp(buf, quit_command, quit_command_len)) {
            if (DEBUG) printf("quit canale\n");
           
            command=1;
                
                         
			my_named_semaphore = sem_open(name_channel, 0); // mode is 0: sem_open is not allowed to create it!
			/**INIZIO SEZIONE CRITICA PER IL CANALE**/
			ret = sem_wait(my_named_semaphore); 
			ERROR_HELPER(ret, "error sem_wait");  
			
			 // controllare se nel frattempo il canale è stato chiuso
			if (leggiMSG(id_coda,&recv_message,key)==0) { 
			
				
				printf("nessun messaggio\n");
			}
			else{				
				esci(recv_message, &is_connect,my_named_semaphore,my_channel,  key);
				continue;
			}       
			
            //chi esce NON è il propretario del canale
            if (my_channel->owner != args->socket_desc) {
                for(i=0; i < my_channel->dim; i++){  
					if(my_channel->client_desc[i] == args->socket_desc){  //trovo il descrittore nel canale
						my_channel->client_desc[i]=my_channel->client_desc[my_channel->dim-1];
						my_channel->client_desc=(int*)realloc(my_channel->client_desc,sizeof(int)*my_channel->dim-1);
						my_channel->dim--;
						my_channel=NULL;
						
						is_connect = 0;  //setto il flag di connessione a false
						invio("quit dal canale\0",args->socket_desc);
						break;
					}					
				}

            } else {
                /**TODO: gestire in caso in cui il creatore esce dal canale**/
                //per ora invio un messaggio al client
                invio("Sei il proprietario non puoi abbandonare il gruppo!\nUsa il comando /delete per cancellare il gruppo ed uscire\0",args->socket_desc);
            }
            ret = sem_post(my_named_semaphore);
			ERROR_HELPER(ret, "error sem_post");
			/**FINE SEZIONE CRITICA PER IL CANALE**/

            
        }
       
        // ****************************   DELETE   ********************************************
        /**    /delete    */
		if (is_connect && recv_bytes == delete_command_len && !memcmp(buf, delete_command, delete_command_len)) {
			if (DEBUG) printf("delete canale\n");
						
			command=1;
			
			my_named_semaphore = sem_open(name_channel, 0); // mode is 0: sem_open is not allowed to create it!
			
			/**INIZIO SEZIONE CRITICA PER IL CANALE**/
			ret = sem_wait(my_named_semaphore); 
			ERROR_HELPER(ret, "error sem_wait"); 
			//solo il proprietario può eliminare il canale
			if (my_channel->owner == args->socket_desc) {	
				
				//tramite la coda di messaggi avverto tutti gli altri thread
				
				mymsg msg;  //struttura per il messaggio da inviare
				msg.mtype=1; //header del messaggio. 1:delete  2:sem_close
				strcpy(msg.mtext,"delete\0");  //testo del messaggio

				for(i=0; i < my_channel->dim; i++){  //inoltro del messaggio escuso se sesso
					if(my_channel->client_desc[i] != args->socket_desc) {	
						int id_coda_other = msgget(my_channel->client_desc[i], IPC_EXCL|0666);  //prendo la coda di messaggi di un client connesso...
						if( id_coda_other == -1 ){
							printf("cannot open server queue, please check with the problem\n");
							ERROR_HELPER(-1,"errore coda");
						}					
						if ( msgsnd(id_coda_other,&msg , SIZE, FLAG) == -1 ) {  //...gli invio il messaggio
							printf("cannot return response to the client\n");
							ERROR_HELPER(-1,"errore coda");
						}else printf("invio a %d al canale %d\n",my_channel->client_desc[i],id_coda_other);			
					}
				}
				
				//ora attendo la conferma che tutti abbiano fatto la sem_close
				//utilizzo il tipo 2 per i messaggi di sem_close
				//aspetto la conferma da tutti (tranne se stesso)
				/*
				for(i=0; i < my_channel->dim; i++){  
					printf("\tscolto su %d\n",id_coda);
					if(my_channel->client_desc[i] != args->socket_desc) {	
						if ( msgrcv(id_coda, &recv_message, sizeof(mymsg), 2, FLAG)  != -1 ) { 
							ERROR_HELPER(-1,"errore coda messaggi");
						}
						else{ 
							if (DEBUG)printf("111111111 asked service of type %ld - receive %s\n", recv_message.mtype, recv_message.mtext); 
							printf("messaggio ricevuto");
						}
					}
				}
				*/
				
				
				
				/**INIZIO SEZIONE CRITICA PER LA LISTA**/
				if(DEBUG)printf("tutti sono usciti\n");
				
				ret = sem_wait(sem);
				ERROR_HELPER(ret, "error sem_wait");
				
				//aggiorno la lista dei canali
				for(i=0;i<args->channel_list->num_channels;i++){
					if(strcmp(args->channel_list->name_channel[i],my_channel->name_channel)==0){
						//tolgo dalla lista il canale
						args->channel_list->channel[i]=args->channel_list->channel[args->channel_list->num_channels-1];
						args->channel_list->channel=(channel_struct**)realloc(args->channel_list->channel, sizeof(channel_struct*)*args->channel_list->num_channels-1);
						
						//tolgo dalla lista il nome
						free(args->channel_list->name_channel[i]); 
						args->channel_list->name_channel[i]=args->channel_list->name_channel[args->channel_list->num_channels-1];
						args->channel_list->name_channel=(char**)realloc(args->channel_list->name_channel, sizeof(char*)*args->channel_list->num_channels-1);
						args->channel_list->num_channels--;
						
					}
				}
				
				//deallocazione risorse canale
				free(my_channel->client_desc);
				//free(my_channel->name_channel);  NON decommentare altrimenti si va in errore
				
				/* ho voluto lasciarlo per essere sicuro di non fare lo stesso errore
				 * SPIEGAZIONE: la stringa che contiene viene puntata anche dal puntatore nella lista dei canali
				 * poichè ho già deallocato questo spazio di memoria qualche riga prima partendo dalla lista sarebbe un errore deallocarla un'altra volta
				 * */	
				 			  
				free(my_channel);
				my_channel=NULL;
				
				/**TODO gestire i ritorni**/
				sem_close(my_named_semaphore);
				sem_unlink(name_channel);
				
				ret = sem_post(sem);
				ERROR_HELPER(ret, "error sem_post");
				/**FINE SEZIONE CRITICA PER LA LISTA**/			
				
				invio("canale eliminato\0",args->socket_desc);
				is_connect=0;
				
		
			}
			else invio("solo il proprietario può eliminare il canale\0",args->socket_desc);
			
			ret = sem_post(my_named_semaphore);
			ERROR_HELPER(ret, "error sem_post");
			/**FINE SEZIONE CRITICA PER IL CANALE**/ 
			
			
	    }
     
     
		        /**TODO: gestire altri comandi (es. /show )**/
		        
       
        // ********************** SEND MESSAGE  *************************************
        if(is_connect && !command){
			/** TODO: decidere cosa fare se inoltra /create e /join */
			if(DEBUG) printf("inoltro\n");
			
			my_named_semaphore = sem_open(name_channel, 0); // mode is 0: sem_open is not allowed to create it!
			/**INIZIO SEZIONE CRITICA PER IL CANALE**/
			ret = sem_wait(my_named_semaphore); 
			ERROR_HELPER(ret, "error sem_wait");  
	
			if ( leggiMSG(id_coda,&recv_message,key)==0){
				//inoltro del messaggio escuso se stesso
				int i=0;
				for(i=0; i < my_channel->dim; i++){  
					if(my_channel->client_desc[i] != args->socket_desc) invio(buf,my_channel->client_desc[i]);
				}
			}
			else {
				esci(recv_message, &is_connect,my_named_semaphore,my_channel,  key);
			}

			ret = sem_post(my_named_semaphore);
			ERROR_HELPER(ret, "error sem_post");
			/**FINE SEZIONE CRITICA PER IL CANALE**/ 
			
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


void invio(char* s, int dest){	
	int ret;
   	while ( (ret = send(dest, s, sizeof(char)*strlen(s)+1, 0)) < 0 ) {
		if (errno == EINTR) continue;
		ERROR_HELPER(-1, "Cannot write to the socket");
	}
}

int ricevi(char* buf,size_t buf_len,int mitt,int id_coda,mymsg* recv_message,int* is_connect,sem_t* my_named_semaphore,channel_struct* my_channel,int key){
	int ret,shouldStop=0;
	int recv_bytes=0;
	
	struct timeval timeout;
    fd_set read_descriptors;
    int nfds = mitt + 1;
    
   
    
    while(!shouldStop){
		// check every 1.5 seconds 
		
		timeout.tv_sec  = 1;
		timeout.tv_usec = 500000;
		
		FD_ZERO(&read_descriptors);
		FD_SET(mitt, &read_descriptors);
		
		
		ret = select(nfds, &read_descriptors, NULL, NULL, &timeout);
		
		printf("sono %d\n",key);
		
		if (ret == -1 && errno == EINTR) continue;
		ERROR_HELPER(ret, "Unable to select()");
	
		
		//controllo periodicamente se è arrivato qualche messaggio
		if (leggiMSG(id_coda,recv_message,key)) {  
			esci(*recv_message, is_connect, my_named_semaphore, my_channel,  key);
		}
		
		if (ret == 0) continue; // timeout expired
		
		printf("è arrivato qualcosa\n");
		
		// ret is 1: read available data!
		while ((recv_bytes = recv(mitt, buf, buf_len, 0)) < 0) {
			if (errno == EINTR) continue;
			ERROR_HELPER(-1, "Cannot read from socket");
		}
		int end=recv_bytes/sizeof(char);
		if(end<buf_len) buf[end]='\0';
		else buf[buf_len]='\0';  //aggiungo il terminatore di stringa manualmente
		shouldStop=1;
		
	}
	
    return recv_bytes;
}


int leggiMSG(int id_coda, mymsg* recv_message, int key){
	//utilizzo il tipo 1 per i messaggi di delete
	
	if ( msgrcv(id_coda, recv_message, sizeof(mymsg), 1, IPC_NOWAIT)  != -1 ) { 
		printf("%d messaggio ricevuto\n",key);
		return 1;  //messaggio ricevuto			
	}
	else if(errno!=ENOMSG){  //ENOMSG: IPC_NOWAIT asserted, and no message exists in the queue to satisfy the request
		ERROR_HELPER(-1,"errore lettura messaggio"); 
		return -1;  //errore!!
	}
	else {
		printf("nessun messaggio %d coda %d\n",key,id_coda);
		return 0;  //nessun messaggio ricevuto
	}
}

void esci(mymsg recv_message, int* is_connect,sem_t* my_named_semaphore,channel_struct* my_channel, int key){
	//il canale sta per essere eliminato quindi esco
	if (DEBUG)printf("asked service of type %ld - receive %s\n", recv_message.mtype, recv_message.mtext); 
	*is_connect=0;
	sem_close(my_named_semaphore);	
/*	
	//avverto il proprietario di aver fatto la sem_close!!
	mymsg msg;
	msg.mtype=2;  //header del messaggio. 1:delete  2:sem_close
	strcpy(msg.mtext,"semclose\0");
	
	int id_coda_other = msgget(my_channel->client_desc[0], IPC_EXCL|0666);  //prendo la coda di messaggi del proprietario...
	if( id_coda_other == -1 ){
		printf("cannot open server queue, please check with the problem\n");
		ERROR_HELPER(-1,"errore coda");
	}					
	if ( msgsnd(id_coda_other,&msg , SIZE, FLAG) == -1 ) {  //...gli invio il messaggio
		printf("cannot return response to the client\n");
		ERROR_HELPER(-1,"errore coda");
	}
	else if (DEBUG)printf("invio a %d of type %ld - receive %s\n", id_coda_other, msg.mtype, msg.mtext); 
*/	
	my_channel=NULL;		
	invio("sei stato disconnesso dal canale\0",key);	//avverto il client che è stato disconnesso dal canale
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



void printList(channel_list_struct* list){
	int i;
	for(i=0;i < list->num_channels;i++){
		printChannel(list->channel[i]);
		printf("\n");
	}
	
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


