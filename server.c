#include "common.h"
#include "log.h"

#include <stdio.h>//printf
#include <stdlib.h>//warning exit
#include <errno.h>//per poter leggere il valore di errno
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>  // htons()
#include <unistd.h> //fork()

#include <fcntl.h>// serve per le opzioni di apertura del file
#include <time.h>//per giorno e data correnti


//#define PTHREAD_ERROR_HELPER(ret, msg)  GENERIC_ERROR_HELPER((ret != 0), ret, msg)



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
    
    if(DEBUG) resetLog(); //resetto il file di log
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
        
        //TODO: fare il controllo tra create or join
        /**SPIEGAZIONE: quando un client si connette al server invierà 2 tipi di richieste:creare un nuovo canale oppure unirsi ad uno esistente
         * se vuole creare un nuovo canale con una fork si crea il processo che se ne occupa (il codice sottostante si occupa di questo)
         * se vuole unirsi ad uno esistente dobbiamo inviare un messaggio al processo del canale (codice non presente)
         * */

        pid_t pid = fork();
        if (pid == -1) {
            ERROR_HELPER(-1, "Cannot fork server process to handle the request");
        } else if (pid == 0) {
            // child: close the listening socket and process the request
            ret = close(socket_desc);
            ERROR_HELPER(ret, "Cannot close listening socket in child process");

            //connection_handler(client_desc, client_addr);
            //

            if (DEBUG) fprintf(stderr, "Process created to handle the request has completed.\n");

            free(client_addr);
            exit(EXIT_SUCCESS);
        } else {
            // server: close the incoming socket and continue
            ret = close(client_desc);
            ERROR_HELPER(ret, "Cannot close incoming socket in main process");

            if (DEBUG) fprintf(stderr, "Child process created to handle the request!\n");

            // reset fields in client_addr
            memset(client_addr, 0, sizeof(struct sockaddr_in));
        }
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
     * expires before any event occurs. */
     
    
