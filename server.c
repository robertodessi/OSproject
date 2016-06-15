#include "common.h"
#include "log.h"
#include "thread.h"


#include <stdio.h>//printf
#include <stdlib.h>//warning exit
#include <errno.h>//per poter leggere il valore di errno
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>  // htons()
#include <unistd.h> 
#include <pthread.h>    
#include <signal.h> //signals handling

#include <sys/types.h>
#include <sys/msg.h>

//  socekt descriptor
int socket_desc, client_desc;

int server_q;

channel_list_struct* channel_list;


void alertThread(){
    int i,ret;
    
    mymsg msgServer; //struttura per il messaggio da inviare
    msgServer.mtype = 3; //header del messaggio. 1:delete  2:sem_close
    strcpy(msgServer.mtext, "killthemall\0"); //testo del messaggio
    
    //mymsg inbox;
    
     
 
    ret=sem_wait(sem);
    if(ret==-1){
	if(DEBUG) printf("errore\n");
    } 
   

 
 
    for(i = 0; i < n_client; i++){
			
            int id_coda_other = msgget(client_sock[i], IPC_EXCL | 0666); //prendo la coda di messaggi di un client connesso...
      
            if (id_coda_other == -1) {
                if(DEBUG) printf("server.c msgget spiacenti, si è verificato un errore\n");
                continue;
            }
          
            if (msgsnd(id_coda_other, &msgServer, SIZE, FLAG) == -1) { //...gli invio il messaggio
                if(DEBUG) printf("cannot return response to the client\n");
                if(DEBUG) printf("server.c msgsend spiacenti, si è verificato un errore\n");
                continue;
            } 
            
    }
   
    ret=sem_post(sem);
    if(ret==-1){
        if(DEBUG) printf("errorepost\n");
    } 
	
  
    
    for(i = 0; i < n_client; i++) pthread_join(threads[i],NULL);
        
    int max = channel_list->num_channels;   
    
    //free e unlink di tutti i canali
    for (i=0;i<max;i++){
		sem_t * channel_semaphore = sem_open(channel_list->channel[i]->name_channel, 0); // prendo il sem del canale
		sem_close(channel_semaphore); 						 //lo chiudo
		sem_unlink(channel_list->channel[i]->name_channel);  //lo unlinko
		free(channel_list->channel[i]->client_desc); 	 //dealloco client_desc
		free(channel_list->channel[i]->name_channel);	 //dealloco il nome del canale
		//free(channel_list->channel[i]->id);				 //dealloco client_desc
		free(channel_list->channel[i]);					//dealloco il canale
	}
	free(channel_list); //dealloco la lista T.T
    
}


void error_handling(int signal, void (*handler)(int, siginfo_t *, void *)) {
    struct sigaction act;
    act.sa_sigaction = handler;
    act.sa_flags = SA_SIGINFO;
    if (sigaction(signal, &act, NULL) == -1) {
        if(DEBUG) printf("Errore Sigaction\n");
        exit(0);
    }
}

void safe_exit(int dummy1, siginfo_t *info, void *dummy2) {
   
    int signo = (int) info->si_signo;
    char * signame;
    switch (signo) {
        case 1:
            signame = "SIGHUP";
            break;
        case 2:
            signame = "SIGINT";
            break;
        case 3:
            signame = "SIGQUIT";
            break;            
        case 4:
            signame = "SIGILL";
            break;            
        case 15:
            signame = "SIGTERM";
            break;  
        default:
            signame = "UKNOWN SIGNAL";
            break;
    }
    
    char buf [64];
    
    strncpy(buf, "\nServer shutting down, signal caught is: ", 60);
    strncat(buf, signame, 15);
   
    
    // fare la write invece che log!
   
    logMsg(buf);
    
    
    //WARNING!! It's unsafe to use logMsg method in a signal handler routine since calls 
    //          the fprintf method and it is not an async-signal safe function
    
    alertThread();
    msgctl(server_q, IPC_RMID, 0);
    fflush(stdout);
    close(socket_desc);
    sem_close(sem);
    sem_unlink(NAME_SEM);

    exit(0);
}


void gestione_sigsegv(int dummy1, siginfo_t *info, void *dummy2) {
    unsigned int address;
    address = *(unsigned int*) &(info->si_addr);
    if(DEBUG) printf("segfault occurred (address is %x)\n", address);
    logSeg(address);
    
    alertThread();
    
    fflush(stdout);
    close(socket_desc);
    sem_close(sem);
    sem_unlink(NAME_SEM);

    exit(0);
}

sigset_t mask;

void sigsegv() {
    struct sigaction act;
    act.sa_sigaction = gestione_sigsegv;
    act.sa_mask = mask;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGSEGV, &act, NULL);
}


//====================================
//			MAIN
//====================================

int main(int argc, char *argv[]) {

    //	error check descriptor 
    int ret;
    

    //  some fields are required to be filled with 0
    //  struct containing server info
    struct sockaddr_in server_addr = {0};
   
    int sockaddr_len = sizeof (struct sockaddr_in); // we will reuse it for accept()
    
    //client_sock=NULL;
    //threads=NULL;
    n_client=0;

    //  struttura che rappresenta la lista di tutti i canali
    channel_list = (channel_list_struct*) malloc(sizeof (channel_list_struct));
    channel_list->num_channels = 0; //inizialmente ci sono 0 canali
    channel_list->name_channel = (char**) malloc(0); //inizializzo le strutture dati
    channel_list->channel = (channel_struct**) malloc(0); //inizializzo le strutture dati

    //  alloco e inizializzo il semaforo
    sem = (sem_t*) malloc(sizeof (sem_t));

    sem = sem_open(NAME_SEM, O_CREAT | O_EXCL, 0666, 1);
    if (sem == SEM_FAILED && errno == EEXIST) {
        logMsg("Global sem already exists, let's unlink it...");
        sem_unlink(NAME_SEM);
        logMsg("and then reopen it!");
        sem = sem_open(NAME_SEM, O_CREAT | O_EXCL, 0666, 1);
    }
    if (sem == SEM_FAILED) {
        ERROR_HELPER(-1, "[FATAL ERROR] Could not create a semaphore");
    }
    //printf("sempahore is %d\n", sem);

    ret = sigemptyset(&mask);
    ERROR_HELPER(ret, "Errore nella sigemptyset\n\n");

    ret = sigfillset(&mask);
    ERROR_HELPER(ret, "Errore nella sigfillset\n\n");

    ret = sigdelset(&mask, SIGTERM);
    ERROR_HELPER(ret, "Errore nella sigdelset(SIGTERM)\n\n");

    ret = sigdelset(&mask, SIGINT);
    ERROR_HELPER(ret, "Errore nella sigdelset(SIGINT)\n\n");

    ret = sigdelset(&mask, SIGQUIT);
    ERROR_HELPER(ret, "Errore nella sigdelset(SIGQUIT)\n\n");

    ret = sigdelset(&mask, SIGHUP);
    ERROR_HELPER(ret, "Errore nella sigdelset(SIGHUP)\n\n");

    ret = sigdelset(&mask, SIGPIPE);
    ERROR_HELPER(ret, "Errore nella sigdelset(SIGPIPE)\n\n");
    
    ret = sigdelset(&mask, SIGSEGV);
    ERROR_HELPER(ret, "Errore nella sigdelset(SIGSEGV)\n\n");
    
    ret = pthread_sigmask(SIG_BLOCK, &mask, NULL);
    ERROR_HELPER(ret, "Errore nella pthread_sigmask\n\n");

    //  ignoring SIGPIPE signal
    if(signal(SIGPIPE, SIG_IGN) == SIG_ERR){
        logMsg("Error ignoring SIGPIPE");
    }
    
    error_handling(SIGTERM, safe_exit);
    error_handling(SIGINT, safe_exit);
    error_handling(SIGQUIT, safe_exit);
    error_handling(SIGHUP, safe_exit);
    error_handling(SIGILL, safe_exit);
    error_handling(SIGSEGV, gestione_sigsegv);
    
    
    //Presentation
    printf("\n\n|==============================Welcome by ChatApp!==============================|\n");

    if (DEBUG) resetLog(); //resetto il file di log
    logMsg("|===================================Server started to run===================================|");

    // initialize socket for listening
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    //socket_desc = -1;
    ERROR_HELPER(socket_desc, "Errore creazione socket\n\n");

    server_addr.sin_addr.s_addr = INADDR_ANY; // we want to accept connections from any interface
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT); // don't forget about network byte order!

    /* We enable SO_REUSEADDR to quickly restart our server after a crash:
     * for more details, read about the TIME_WAIT state in the TCP protocol */
    int reuseaddr_opt = 1;
    ret = setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_opt, sizeof (reuseaddr_opt));
    ERROR_HELPER(ret, "Cannot set SO_REUSEADDR option");

    // bind address to socket
    ret = bind(socket_desc, (struct sockaddr*) &server_addr, sockaddr_len);
    ERROR_HELPER(ret, "Cannot bind address to socket");

    // start listening
    ret = listen(socket_desc, MAX_CONN_QUEUE);
    ERROR_HELPER(ret, "Cannot listen on socket");

    // we allocate client_addr dynamically and initialize it to zero
    struct sockaddr_in* client_addr = calloc(1, sizeof (struct sockaddr_in));
    
    server_q = msgget(socket_desc, IPC_CREAT | 0666);
    if (server_q == -1) {
        printf("cannot install server queue (server)\n");
        printf("eccoloserver: %s\n", strerror(errno));
    }


    printf("server queue is %d\n\n", server_q);
    
    // loop to manage incoming connections forking the server process
    while (1) {
        // accept incoming connection
        client_desc = accept(socket_desc, (struct sockaddr*) client_addr, (socklen_t*) & sockaddr_len);
        if (client_desc == -1 && errno == EINTR) continue; // check for interruption by signals
        if (client_desc == -1) {
            logMsg("Cannot open socket for incoming connection, retrying...");
            continue;
        }
        
        // parse client IP address and port
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr->sin_addr), client_ip, INET_ADDRSTRLEN);
        uint16_t client_port = ntohs(client_addr->sin_port); // port number is an unsigned short

        if (DEBUG) fprintf(stderr, "Incoming connection accepted... %s:%d\n", client_ip, client_port);

        logConnection(client_ip, client_port);

        //  creo il thread che gestirà il client da ora in avanti
        pthread_t thread;
		
        
        // put arguments for the new thread into a buffer
        handler_args_t * thread_args = (handler_args_t*) malloc(sizeof (handler_args_t));
        thread_args -> socket_desc = client_desc; //passo il descrittore del client
        thread_args -> client_addr = client_addr; //passo l'indirizzo del client
        thread_args -> channel_list = channel_list; //passo il puntatore alla lista dei canali
        thread_args -> server_sd = socket_desc;


        if (pthread_create(&thread, NULL, connection_handler, (void*) thread_args) != 0) {
            logError("Can't create a new thread", errno);
            fprintf(stderr, "Can't create a new thread, error %d\n", errno);
            exit(EXIT_FAILURE);
        }
		
		//n_client++;
		//threads=(pthread_t*)realloc(threads,n_client);
		//threads[n_client-1]=thread;
		//client_sock=(int*)realloc(client_sock,n_client);
		//client_sock[n_client-1]=client_desc;
		
        if (DEBUG) fprintf(stderr, "New thread created to handle the request!\n");
        
        logMsg("New thread created to handle the request!");
        

        
        //pthread_detach(thread); // I won't phtread_join() on this thread

        // we can't just reset fields: we need a new buffer for client_addr!
        client_addr = calloc(1, sizeof (struct sockaddr_in));

    }

    exit(EXIT_SUCCESS); // this will never be executed
    
}
